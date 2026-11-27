// Copyright (c) 2026 Heritage VR contributors. All rights reserved.

#include "HeritageVRGameMode.h"
#include "Engine/Light.h"
#include "EngineUtils.h"
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

	int32 NumDirectional = 0;
	int32 NumSky = 0;
	float SunIntensity = -1.f;
	for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
	{
		++NumDirectional;
		SunIntensity = It->GetLightComponent()->Intensity;
	}
	for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
	{
		++NumSky;
	}

	const FString LightsSummary = FString::Printf(
		TEXT("HeritageVR | map: %s | RHI: %s | lumieres: dir=%d (intensite=%.1f) sky=%d"),
		*GetWorld()->GetMapName(),
		GDynamicRHI ? *GDynamicRHI->GetName() : TEXT("null"),
		NumDirectional, SunIntensity, NumSky);
	UE_LOG(LogTemp, Log, TEXT("%s"), *LightsSummary);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2, 15.f, FColor::Cyan, LightsSummary);
	}
}
