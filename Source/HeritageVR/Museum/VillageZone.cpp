// Copyright (c) 2026 Heritage VR contributors. All rights reserved.

#include "Museum/VillageZone.h"

#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"

AVillageZone::AVillageZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneVolume"));
	RootComponent = ZoneVolume;
	ZoneVolume->InitBoxExtent(FVector(800.f, 800.f, 300.f));
	ZoneVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void AVillageZone::BeginPlay()
{
	Super::BeginPlay();

	ZoneVolume->OnComponentBeginOverlap.AddDynamic(this, &AVillageZone::HandleOverlapBegin);
	ZoneVolume->OnComponentEndOverlap.AddDynamic(this, &AVillageZone::HandleOverlapEnd);

	if (!AmbienceSound.IsValid())
	{
		return;
	}

	if (UAudioComponent* Ambience = NewObject<UAudioComponent>(this))
	{
		if (USoundBase* Sound = Cast<USoundBase>(AmbienceSound.TryLoad()))
		{
			Ambience->SetSound(Sound);
			Ambience->bIsUISound = false;
			Ambience->bAutoActivate = true;
			Ambience->RegisterComponentWithWorld(GetWorld());
		}
	}
}

void AVillageZone::ConfigureZone(const FName& InVillageId, const FText& InDisplayName, const FText& InDescriptionFr)
{
	VillageId = InVillageId;
	DisplayName = InDisplayName;
	DescriptionFr = InDescriptionFr;
}

void AVillageZone::HandleOverlapBegin(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (OtherActor && OtherActor->IsA<APawn>())
	{
		OnVillageEntered.Broadcast(this);
	}
}

void AVillageZone::HandleOverlapEnd(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32)
{
	if (OtherActor && OtherActor->IsA<APawn>())
	{
		OnVillageExited.Broadcast(this);
	}
}
