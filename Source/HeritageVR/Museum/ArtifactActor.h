#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Museum/ArtifactInfo.h"
#include "Interaction/Interactable.h"
#include "ArtifactActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UAudioComponent;
class UMotionControllerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArtifactExamined, class AArtifactActor*, Artifact);

/** Objet culturel 3D manipulable : masque, statuette, tissu, instrument. */
UCLASS(Blueprintable)
class HERITAGEVR_API AArtifactActor : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AArtifactActor();

	virtual void Tick(float DeltaSeconds) override;

	// --- IInteractable ---
	virtual void Grab_Implementation(UMotionControllerComponent* Hand) override;
	virtual void Release_Implementation(UMotionControllerComponent* Hand) override;
	virtual void SetFocused_Implementation(bool bFocused) override;

	/** Joue la narration dans la langue demandee ("fr" ou code de langue locale). */
	UFUNCTION(BlueprintCallable, Category = "Artifact|Audio")
	void PlayNarration(const FString& LanguageCode);

	UFUNCTION(BlueprintCallable, Category = "Artifact")
	void ApplyArtifactInfo(const FArtifactInfo& InInfo);

	UPROPERTY(BlueprintAssignable, Category = "Artifact")
	FOnArtifactExamined OnArtifactExamined;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Artifact")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Zone de saisie et de focus autour de l'objet. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Artifact")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact")
	FArtifactInfo Info;

	/** Stencil de custom depth pour l'outline Blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Artifact")
	int32 FocusStencilValue = 1;

private:
	void UpdateExamination();

	UPROPERTY()
	TObjectPtr<UAudioComponent> NarrationComponent;

	bool bExamined = false;
	bool bHeldByHand = false;
	float TimeFocused = 0.f;
};
