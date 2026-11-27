#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

class UMotionControllerComponent;

UINTERFACE(MinimalAPI, BlueprintType)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/** Contrat commun des objets manipulables en VR (artefacts, panneaux, leviers). */
class HERITAGEVR_API IInteractable
{
	GENERATED_BODY()

public:
	/** Saisir l'objet avec la main donnee. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Grab(UMotionControllerComponent* Hand);

	/** Relacher l'objet. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Release(UMotionControllerComponent* Hand);

	/** Highlight visuel quand la main vise l'objet. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void SetFocused(bool bFocused);
};
