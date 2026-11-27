#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioCaptureCore.h"
#include "HttpFwd.h"
#include "VoiceAssistantSubsystem.generated.h"

class UAudioComponent;
class USoundWaveProcedural;

/** Langues supportees par l'assistant du musee. */
UENUM(BlueprintType)
enum class EAssistantLanguage : uint8
{
	French,
	Baoule,
	Dioula,
	Bete,
	Senoufo,
	Yacouba
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssistantStateChanged, bool, bListening);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssistantTranscription, const FText&, QuestionText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssistantReply, const FText&, ReplyText);

/**
 * Assistant vocal du musee.
 * Pipeline : micro (AudioCaptureCore, 16 kHz mono) -> WAV -> STT (HTTP) -> LLM (HTTP) -> TTS (HTTP) -> lecture (SoundWaveProcedural).
 * Endpoints configures dans Config/DefaultGame.ini (section [/Script/HeritageVR.VoiceAssistantSubsystem]).
 * Les secrets (cles d'API) ne sont jamais stockes dans le depot : voir docs/VOICE_ASSISTANT.md.
 */
UCLASS(Config = Game)
class HERITAGEVR_API UVoiceAssistantSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetLanguage(EAssistantLanguage Language);

	UFUNCTION(BlueprintPure, Category = "Voice")
	EAssistantLanguage GetLanguage() const;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void StartListening();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void StopListeningAndSubmit();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void AskQuestion(const FText& Question);

	/** Lit un texte pre-existant (phrase pre-enregistree du musee par exemple). */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SpeakText(const FText& TextToSpeak);

	UPROPERTY(BlueprintAssignable, Category = "Voice")
	FOnAssistantStateChanged OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Voice")
	FOnAssistantTranscription OnTranscription;

	UPROPERTY(BlueprintAssignable, Category = "Voice")
	FOnAssistantReply OnReply;

private:
	FString ResolveLanguageCode() const;

	void SendTranscriptionRequest(const TArray<uint8>& WavBytes);
	void SendLlmRequest(const FString& QuestionText);
	void SendTtsRequest(const FString& ReplyText);

	void HandleTranscriptionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleLlmResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleTtsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

	void QueueReplyForPlayback(const TArray<uint8>& AudioBytes, int32 SampleRate);
	void ResetCapture();

	static void BuildWavHeader(TArray<uint8>& OutWav, const TArray<int16>& PcmSamples, int32 SampleRate, int32 NumChannels);

	UPROPERTY(Config, EditAnywhere, Category = "Voice|Service")
	FString SttEndpointUrl;

	UPROPERTY(Config, EditAnywhere, Category = "Voice|Service")
	FString LlmEndpointUrl;

	UPROPERTY(Config, EditAnywhere, Category = "Voice|Service")
	FString TtsEndpointUrl;

	Audio::FAudioCapture Capture;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> PlaybackComponent;

	UPROPERTY(Transient)
	TObjectPtr<USoundWaveProcedural> PlaybackWave;

	FCriticalSection CaptureSection;
	TArray<int16> CapturedSamples;
	int32 CapturedSampleRate = 16000;
	EAssistantLanguage CurrentLanguage = EAssistantLanguage::French;
	bool bListening = false;
};
