// Copyright (c) 2026 Heritage VR contributors. All rights reserved.

#include "Player/HeritageVRPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MotionControllerComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Interaction/Interactable.h"

namespace
{
	constexpr float kFloorTraceDistance = 300.f;
}

AHeritageVRPawn::AHeritageVRPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	SetRootComponent(VRRoot);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(VRRoot);
	Capsule->SetCapsuleRadius(30.f);
	Capsule->SetCapsuleHalfHeight(90.f);
	Capsule->SetCollisionProfileName(TEXT("Pawn"));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	LeftHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(VRRoot);
	LeftHand->SetTrackingSource(EControllerHand::Left);
	LeftHand->SetShowDeviceModel(true);

	RightHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHand"));
	RightHand->SetupAttachment(VRRoot);
	RightHand->SetTrackingSource(EControllerHand::Right);
	RightHand->SetShowDeviceModel(true);
}

void AHeritageVRPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AHeritageVRPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// La capsule suit la position reelle du casque pour les volumes (zones, portails).
	if (Camera && Capsule)
	{
		Capsule->SetWorldLocation(Camera->GetComponentLocation());
	}

	// Diagnostique affiché à l'écran : confirme que le pion et la caméra sont actifs
	// même si la scène s'affiche noir (le texte passe au-dessus du rendu).
	if (GEngine && Camera && GetWorld() && GetWorld()->GetTimeSeconds() >= NextDebugUpdateTime)
	{
		NextDebugUpdateTime = GetWorld()->GetTimeSeconds() + 1.0;
		GEngine->AddOnScreenDebugMessage(1, 1.2f, FColor::Yellow, FString::Printf(
			TEXT("HeritageVR | cam: %s %s | HMD: %s"),
			*Camera->GetComponentLocation().ToCompactString(),
			*Camera->GetComponentRotation().ToCompactString(),
			UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayConnected() ? TEXT("connecte") : TEXT("desktop")));
	}
}

void AHeritageVRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Teleport", IE_Pressed, this, &AHeritageVRPawn::OnTeleportPressed);
	PlayerInputComponent->BindAction("Teleport", IE_Released, this, &AHeritageVRPawn::OnTeleportReleased);

	PlayerInputComponent->BindAction("GrabRight", IE_Pressed, this, &AHeritageVRPawn::OnGrabRightPressed);
	PlayerInputComponent->BindAction("GrabRight", IE_Released, this, &AHeritageVRPawn::OnGrabRightReleased);
	PlayerInputComponent->BindAction("GrabLeft", IE_Pressed, this, &AHeritageVRPawn::OnGrabLeftPressed);
	PlayerInputComponent->BindAction("GrabLeft", IE_Released, this, &AHeritageVRPawn::OnGrabLeftReleased);

	PlayerInputComponent->BindAction("TurnLeft", IE_Pressed, this, &AHeritageVRPawn::OnTurnLeft);
	PlayerInputComponent->BindAction("TurnRight", IE_Pressed, this, &AHeritageVRPawn::OnTurnRight);
}

void AHeritageVRPawn::OnTeleportPressed()
{
	// L'arc de visée (spline + matériau) est ajouté côté Blueprint ; seule la destination est calculée ici.
}

void AHeritageVRPawn::OnTeleportReleased()
{
	FVector Destination;
	if (TryGetTeleportDestination(Destination))
	{
		ExecuteTeleport(Destination);
	}
}

void AHeritageVRPawn::OnTurnLeft()
{
	PerformSnapTurn(-SnapTurnAngle);
}

void AHeritageVRPawn::OnTurnRight()
{
	PerformSnapTurn(SnapTurnAngle);
}

void AHeritageVRPawn::OnGrabRightPressed()
{
	OnGrabPressed(RightHand);
}

void AHeritageVRPawn::OnGrabRightReleased()
{
	OnGrabReleased(RightHand);
}

void AHeritageVRPawn::OnGrabLeftPressed()
{
	OnGrabPressed(LeftHand);
}

void AHeritageVRPawn::OnGrabLeftReleased()
{
	OnGrabReleased(LeftHand);
}

void AHeritageVRPawn::OnGrabPressed(UMotionControllerComponent* Hand)
{
	if (!Hand)
	{
		return;
	}

	UInteractable* Candidate = FindInteractableNearHand(Hand);
	if (!Candidate)
	{
		return;
	}

	Candidate->SetFocused(true);
	Candidate->Grab(Hand);
	GrabbedObject = Candidate;
	GrabbingHand = Hand;
}

void AHeritageVRPawn::OnGrabReleased(UMotionControllerComponent* Hand)
{
	if (GrabbedObject.IsValid() && GrabbingHand.Get() == Hand)
	{
		GrabbedObject->Release(Hand);
		GrabbedObject = nullptr;
		GrabbingHand = nullptr;
	}
}

bool AHeritageVRPawn::TryGetTeleportDestination(FVector& OutLocation) const
{
	const UWorld* World = GetWorld();
	if (!World || !Camera)
	{
		return false;
	}

	const FVector CameraLocation = Camera->GetComponentLocation();
	FHitResult FloorHit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(HeritageVRTeleport), /*bTraceComplex*/ false);
	if (!World->LineTraceSingleByChannel(FloorHit, CameraLocation, CameraLocation - FVector::UpVector * kFloorTraceDistance, ECC_Visibility, Params))
	{
		return false;
	}

	OutLocation = FloorHit.ImpactPoint;
	return true;
}

UInteractable* AHeritageVRPawn::FindInteractableNearHand(UMotionControllerComponent* Hand) const
{
	const UWorld* World = GetWorld();
	if (!World || !Hand)
	{
		return nullptr;
	}

	const FVector HandLocation = Hand->GetComponentLocation();
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(HeritageVRGrab), /*bTraceComplex*/ false);
	Params.AddIgnoredActor(this);
	World->OverlapMultiByChannel(Overlaps, HandLocation, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(GrabRadius), Params);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* OverlappedActor = Overlap.GetActor();
		if (!OverlappedActor || OverlappedActor == this)
		{
			continue;
		}
		if (UInteractable* Candidate = Cast<UInteractable>(OverlappedActor))
		{
			return Candidate;
		}
	}
	return nullptr;
}

void AHeritageVRPawn::PerformSnapTurn(float Degrees)
{
	if (!Camera)
	{
		return;
	}

	const FVector CameraWorldLocation = Camera->GetComponentLocation();
	const FQuat TurnQuat(FVector::UpVector, FMath::DegreesToRadians(Degrees));

	const FVector ActorLocation = GetActorLocation();
	SetActorLocation(CameraWorldLocation + TurnQuat.RotateVector(ActorLocation - CameraWorldLocation));
	AddActorWorldRotation(FRotator(0.f, Degrees, 0.f));
}

void AHeritageVRPawn::ExecuteTeleport(const FVector& Destination)
{
	const FVector CameraOffset = Camera ? (Camera->GetComponentLocation() - GetActorLocation()) : FVector::ZeroVector;
	const FVector TargetLocation = Destination - FVector(CameraOffset.X, CameraOffset.Y, 0.f);
	SetActorLocation(TargetLocation, /*bSweep*/ false);
}
