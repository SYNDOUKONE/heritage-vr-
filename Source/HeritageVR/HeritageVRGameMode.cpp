// Copyright (c) 2026 Heritage VR contributors. All rights reserved.

#include "HeritageVRGameMode.h"
#include "Player/HeritageVRPawn.h"

AHeritageVRGameMode::AHeritageVRGameMode()
{
	DefaultPawnClass = AHeritageVRPawn::StaticClass();
	PrimaryActorTick.bCanEverTick = false;
}
