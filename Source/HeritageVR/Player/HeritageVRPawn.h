#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HeritageVRPawn.generated.h"

class UCameraComponent;
class UMotionControllerComponent;

/** Pion VR du visiteur : teleportation, snap turn, saisie d'artefacts. */
UCLASS()
class HERITAGEVR_API AHeritageVRPawn : public APawn
{
	GENERATED_BODY()

public:
	AHeritageVRPawn();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	/** Origine du rig VR, suivie par le casque via le tracking OpenXR. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	TObjectPtr<USceneComponent> VRRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	TObjectPtr<UMotionControllerComponent> LeftHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	TObjectPtr<UMotionControllerComponent> RightHand;

	/** Angle d'un snap turn, en degres. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Locomotion", meta = (ClampMin = "15.0", ClampMax = "90.0"))
	float SnapTurnAngle = 30.f;

	/** Rayon du trace de saisie autour de la main. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Interaction", meta = (ClampMin = "5.0", ClampMax = "100.0"))
	float GrabRadius = 15.f;

	/** Duree du fondu lors d'une teleportation, en secondes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Locomotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TeleportFadeDuration = 0.2f;

private:
	void OnTeleportPressed();
	void OnTeleportReleased();
	void OnTurnLeft();
	void OnTurnRight();
	void OnGrabRightPressed();
	void OnGrabRightReleased();
	void OnGrabLeftPressed();
	void OnGrabLeftReleased();
	void OnGrabPressed(UMotionControllerComponent* Hand);
	void OnGrabReleased(UMotionControllerComponent* Hand);

	bool TryGetTeleportDestination(FVector& OutLocation) const;
	AActor* FindInteractableNearHand(UMotionControllerComponent* Hand) const;
	void PerformSnapTurn(float Degrees);
	void ExecuteTeleport(const FVector& Destination);

	TWeakObjectPtr<UMotionControllerComponent> GrabbingHand;
	TWeakObjectPtr<AActor> GrabbedObject;
};
