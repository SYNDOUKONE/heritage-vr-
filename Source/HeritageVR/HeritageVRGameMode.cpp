// Copyright (c) 2026 Heritage VR contributors. All rights reserved.

#include "HeritageVRGameMode.h"
#include "Player/HeritageVRPawn.h"
#include "DynamicRHI.h"

AHeritageVRGameMode::AHeritageVRGameMode()
{
	DefaultPawnClass = AHeritageVRPawn::StaticClass();
	PrimaryActorTick.bCanEverTick = false;
}

void AHeritageVRGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("HeritageVR: map=%s pawn=%s RHI=%s"),
		*GetWorld()->GetMapName(),
		DefaultPawnClass ? *DefaultPawnClass->GetName() : TEXT("null"),
		GDynamicRHI ? *GDynamicRHI->GetName() : TEXT("null"));
}
