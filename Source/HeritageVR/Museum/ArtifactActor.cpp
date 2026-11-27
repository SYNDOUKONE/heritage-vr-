// Copyright (c) 2026 Heritage VR contributors. All rights reserved.

#include "Museum/ArtifactActor.h"

#include "Components/AudioComponent.h"
#include "Components/MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundBase.h"

namespace
{
	constexpr float kExaminationFocusSeconds = 1.5f;
}

AArtifactActor::AArtifactActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->InitSphereRadius(25.f);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AArtifactActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateExamination();
}

void AArtifactActor::Grab_Implementation(UMotionControllerComponent* Hand)
{
	if (!Hand || !Mesh)
	{
		return;
	}

	Mesh->SetSimulatePhysics(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->AttachToComponent(Hand, FAttachmentTransformRules::KeepWorldTransform);
	bHeldByHand = true;
}

void AArtifactActor::Release_Implementation(UMotionControllerComponent* Hand)
{
	if (!Mesh)
	{
		return;
	}

	Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetSimulatePhysics(false);
	bHeldByHand = false;
}

void AArtifactActor::SetFocused_Implementation(bool bFocused)
{
	if (!Mesh)
	{
		return;
	}

	Mesh->SetRenderCustomDepth(bFocused);
	Mesh->SetCustomDepthStencilValue(FocusStencilValue);
	if (!bFocused)
	{
		TimeFocused = 0.f;
	}
}

void AArtifactActor::PlayNarration(const FString& LanguageCode)
{
	const FSoftObjectPath& Path = (LanguageCode.Equals(TEXT("fr"), ESearchCase::IgnoreCase))
		? Info.NarrationAudioFr
		: Info.NarrationAudioLocal;
	if (!Path.IsValid())
	{
		return;
	}

	if (!NarrationComponent)
	{
		NarrationComponent = NewObject<UAudioComponent>(this);
		NarrationComponent->SetupAttachment(RootComponent);
	}

	if (USoundBase* Sound = Cast<USoundBase>(Path.TryLoad()))
	{
		NarrationComponent->SetSound(Sound);
		NarrationComponent->Play();
	}
}

void AArtifactActor::ApplyArtifactInfo(const FArtifactInfo& InInfo)
{
	Info = InInfo;
}

void AArtifactActor::UpdateExamination()
{
	if (bExamined || bHeldByHand)
	{
		return;
	}

	const bool bMeshRenderCustomDepth = Mesh && Mesh->bRenderCustomDepth;
	TimeFocused = bMeshRenderCustomDepth ? TimeFocused + GetWorld()->GetDeltaSeconds() : 0.f;

	if (TimeFocused >= kExaminationFocusSeconds)
	{
		bExamined = true;
		OnArtifactExamined.Broadcast(this);
	}
}
