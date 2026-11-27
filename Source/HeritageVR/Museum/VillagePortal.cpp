// Copyright (c) 2026 Heritage VR contributors. All rights reserved.

#include "Museum/VillagePortal.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

AVillagePortal::AVillagePortal()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	RootComponent = TriggerVolume;
	TriggerVolume->InitBoxExtent(FVector(120.f, 120.f, 200.f));
	TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void AVillagePortal::BeginPlay()
{
	Super::BeginPlay();
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AVillagePortal::HandleOverlapBegin);
}

void AVillagePortal::ConfigurePortal(FName InTargetLevelName, const FText& InLabel)
{
	TargetLevelName = InTargetLevelName;
	PortalLabel = InLabel;
}

void AVillagePortal::HandleOverlapBegin(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (bPendingTravel || !OtherActor || !OtherActor->IsA<APawn>())
	{
		return;
	}

	bPendingTravel = true;
	UE_LOG(LogTemp, Log, TEXT("VillagePortal: voyage vers %s"), *TargetLevelName.ToString());
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &AVillagePortal::DoOpenLevel));
}

void AVillagePortal::DoOpenLevel()
{
	if (!TargetLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, TargetLevelName);
	}
}
