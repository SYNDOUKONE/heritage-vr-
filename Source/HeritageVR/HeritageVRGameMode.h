#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HeritageVRGameMode.generated.h"

/** Mode de jeu du musee : installe le pion VR et recolte la progression du visiteur. */
UCLASS()
class HERITAGEVR_API AHeritageVRGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHeritageVRGameMode();
};
