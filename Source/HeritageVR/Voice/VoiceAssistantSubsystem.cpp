// Copyright (c) 2026 Heritage VR contributors. All rights reserved.

#include "Voice/VoiceAssistantSubsystem.h"

#include "Components/AudioComponent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Sound/SoundWaveProcedural.h"

namespace
{
	constexpr int32 kSampleRate = 16000;
	constexpr int32 kNumChannels = 1;
	const TCHAR* kLanguageCodes[] = { TEXT("fr"), TEXT("baoule"), TEXT("dyu"), TEXT("bete"), TEXT("senoufo"), TEXT("yacouba") };
}

void UVoiceAssistantSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadConfig();
}

void UVoiceAssistantSubsystem::Deinitialize()
{
	ResetCapture();
	Super::Deinitialize();
}

FString UVoiceAssistantSubsystem::ResolveLanguageCode() const
{
	return kLanguageCodes[static_cast<int32>(CurrentLanguage)];
}

void UVoiceAssistantSubsystem::SetLanguage(EAssistantLanguage Language)
{
	CurrentLanguage = Language;
}

EAssistantLanguage UVoiceAssistantSubsystem::GetLanguage() const
{
	return CurrentLanguage;
}

void UVoiceAssistantSubsystem::StartListening()
{
	if (bListening)
	{
		return;
	}

	ResetCapture();
	CapturedSamples.Reset();

	Audio::FAudioCaptureDeviceParams CaptureParams;
	if (!Capture.OpenAudioCaptureStream(CaptureParams, [this](const void* Buffer, int32 NumFrames, int32 NumChannels, int32 SampleRate, double, bool)
	{
		if (!Buffer || NumChannels <= 0)
		{
			return;
		}
		const float* Samples = static_cast<const float*>(Buffer);
		CapturedSampleRate = SampleRate;
		CaptureSection.Lock();
		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float Clamped = FMath::Clamp(Samples[Frame * NumChannels], -1.f, 1.f);
			CapturedSamples.Add(static_cast<int16>(Clamped * static_cast<float>(TNumericLimits<int16>::Max())));
		}
		CaptureSection.Unlock();
	}, 1024))
	{
		UE_LOG(LogTemp, Warning, TEXT("VoiceAssistant: ouverture du flux micro impossible"));
		return;
	}

	if (!Capture.StartStream())
	{
		UE_LOG(LogTemp, Warning, TEXT("VoiceAssistant: demarrage du flux micro impossible"));
		ResetCapture();
		return;
	}
	bListening = true;
	OnStateChanged.Broadcast(true);
}

void UVoiceAssistantSubsystem::StopListeningAndSubmit()
{
	if (!bListening || !Capture.IsCapturing())
	{
		return;
	}

	Capture.StopStream();
	Capture.CloseStream();
	bListening = false;
	OnStateChanged.Broadcast(false);

	TArray<int16> SamplesCopy;
	CaptureSection.Lock();
	SamplesCopy = MoveTemp(CapturedSamples);
	CaptureSection.Unlock();

	if (SamplesCopy.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("VoiceAssistant: aucune capture audio a transmettre"));
		return;
	}

	TArray<uint8> WavBytes;
	BuildWavHeader(WavBytes, SamplesCopy, CapturedSampleRate, kNumChannels);
	SendTranscriptionRequest(WavBytes);
}

void UVoiceAssistantSubsystem::AskQuestion(const FText& Question)
{
	const FString QuestionText = Question.ToString();
	if (QuestionText.IsEmpty())
	{
		return;
	}
	OnTranscription.Broadcast(Question);
	SendLlmRequest(QuestionText);
}

void UVoiceAssistantSubsystem::SpeakText(const FText& TextToSpeak)
{
	SendTtsRequest(TextToSpeak.ToString());
}

void UVoiceAssistantSubsystem::SendTranscriptionRequest(const TArray<uint8>& WavBytes)
{
	if (SttEndpointUrl.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("VoiceAssistant: SttEndpointUrl non configure"));
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb(TEXT("POST"));
	Request->SetURL(SttEndpointUrl);
	Request->SetHeader(TEXT("Content-Type"), TEXT("audio/wav"));
	Request->SetHeader(TEXT("X-Audio-Language"), ResolveLanguageCode());
	Request->SetContent(WavBytes);
	Request->OnProcessRequestComplete().BindUObject(this, &UVoiceAssistantSubsystem::HandleTranscriptionResponse);
	Request->ProcessRequest();
}

void UVoiceAssistantSubsystem::SendLlmRequest(const FString& QuestionText)
{
	if (LlmEndpointUrl.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("VoiceAssistant: LlmEndpointUrl non configure"));
		return;
	}

	const FString Payload = FString::Printf(
		TEXT(R"({"language":"%s","question":"%s"})"),
		*ResolveLanguageCode(),
		*QuestionText.ReplaceCharWithEscapedChar().Replace(TEXT("\""), TEXT("\\\"")));

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb(TEXT("POST"));
	Request->SetURL(LlmEndpointUrl);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UVoiceAssistantSubsystem::HandleLlmResponse);
	Request->ProcessRequest();
}

void UVoiceAssistantSubsystem::SendTtsRequest(const FString& ReplyText)
{
	if (TtsEndpointUrl.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("VoiceAssistant: TtsEndpointUrl non configure"));
		return;
	}

	const FString Payload = FString::Printf(
		TEXT(R"({"language":"%s","text":"%s"})"),
		*ResolveLanguageCode(),
		*ReplyText.ReplaceCharWithEscapedChar().Replace(TEXT("\""), TEXT("\\\"")));

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb(TEXT("POST"));
	Request->SetURL(TtsEndpointUrl);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UVoiceAssistantSubsystem::HandleTtsResponse);
	Request->ProcessRequest();
}

void UVoiceAssistantSubsystem::HandleTranscriptionResponse(FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
{
	if (!bSucceeded || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Warning, TEXT("VoiceAssistant: echec de la transcription STT"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return;
	}

	const FString Question = JsonObject->GetStringField(TEXT("text"));
	if (Question.IsEmpty())
	{
		return;
	}

	AskQuestion(FText::FromString(Question));
}

void UVoiceAssistantSubsystem::HandleLlmResponse(FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
{
	if (!bSucceeded || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Warning, TEXT("VoiceAssistant: echec de la reponse LLM"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return;
	}

	const FString Reply = JsonObject->GetStringField(TEXT("reply"));
	if (Reply.IsEmpty())
	{
		return;
	}

	OnReply.Broadcast(FText::FromString(Reply));
	SendTtsRequest(Reply);
}

void UVoiceAssistantSubsystem::HandleTtsResponse(FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
{
	if (!bSucceeded || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Warning, TEXT("VoiceAssistant: echec de la synthese TTS"));
		return;
	}

	const TArray<uint8> AudioBytes = Response->GetContent();
	if (AudioBytes.Num() < 2)
	{
		return;
	}

	const FString SampleRateHeader = Response->GetHeader(TEXT("X-Audio-Sample-Rate"));
	int32 SampleRate = kSampleRate;
	if (!SampleRateHeader.IsEmpty() && SampleRateHeader.IsNumeric())
	{
		SampleRate = FCString::Atoi(*SampleRateHeader);
	}

	QueueReplyForPlayback(AudioBytes, SampleRate);
}

void UVoiceAssistantSubsystem::QueueReplyForPlayback(const TArray<uint8>& AudioBytes, int32 SampleRate)
{
	PlaybackWave = NewObject<USoundWaveProcedural>();
	PlaybackWave->SetSampleRate(SampleRate > 0 ? SampleRate : kSampleRate);
	PlaybackWave->NumChannels = kNumChannels;
	PlaybackWave->SoundGroup = SOUNDGROUP_Voice;
	PlaybackWave->bLooping = false;

	const int32 NumSamples = AudioBytes.Num() / sizeof(int16);
	PlaybackWave->Duration = static_cast<float>(NumSamples) / static_cast<float>(PlaybackWave->GetSampleRateForCurrentPlatform());
	PlaybackWave->QueueAudio(AudioBytes.GetData(), AudioBytes.Num());

	if (!PlaybackComponent)
	{
		PlaybackComponent = NewObject<UAudioComponent>(this);
		PlaybackComponent->bIsUISound = true;
		PlaybackComponent->RegisterComponentWithWorld(GetWorld());
	}

	PlaybackComponent->SetSound(PlaybackWave);
	PlaybackComponent->Play();
}

void UVoiceAssistantSubsystem::ResetCapture()
{
	if (Capture.IsStreamOpen())
	{
		if (Capture.IsCapturing())
		{
			Capture.StopStream();
		}
		Capture.CloseStream();
	}
	bListening = false;
}

void UVoiceAssistantSubsystem::BuildWavHeader(TArray<uint8>& OutWav, const TArray<int16>& PcmSamples, int32 SampleRate, int32 NumChannels)
{
	const int32 DataBytes = PcmSamples.Num() * sizeof(int16);
	const int32 FileBytes = 44 + DataBytes;

	OutWav.Reset(FileBytes);
	auto AppendFourCC = [&OutWav](const char* Tag)
	{
		OutWav.Append(reinterpret_cast<const uint8*>(Tag), 4);
	};
	auto AppendUint32 = [&OutWav](uint32 Value)
	{
		OutWav.Append(reinterpret_cast<const uint8*>(&Value), sizeof(uint32));
	};
	auto AppendUint16 = [&OutWav](uint16 Value)
	{
		OutWav.Append(reinterpret_cast<const uint8*>(&Value), sizeof(uint16));
	};

	AppendFourCC("RIFF");
	AppendUint32(static_cast<uint32>(FileBytes - 8));
	AppendFourCC("WAVE");
	AppendFourCC("fmt ");
	AppendUint32(16);
	AppendUint16(1);
	AppendUint16(static_cast<uint16>(NumChannels));
	AppendUint32(static_cast<uint32>(SampleRate));
	AppendUint32(static_cast<uint32>(SampleRate * NumChannels * sizeof(int16)));
	AppendUint16(static_cast<uint16>(NumChannels * sizeof(int16)));
	AppendUint16(16);
	AppendFourCC("data");
	AppendUint32(static_cast<uint32>(DataBytes));
	OutWav.Append(reinterpret_cast<const uint8*>(PcmSamples.GetData()), DataBytes);
}
