#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VillagePortal.generated.h"

class UBoxComponent;

/** Portail : passage d'un niveau du musee a un autre en entrant dans le volume. */
UCLASS()
class HERITAGEVR_API AVillagePortal : public AActor
{
	GENERATED_BODY()

public:
	AVillagePortal();

	/** Nom du niveau destination (path long, ex. /Game/Maps/Villages/Baoule_Village). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	FName TargetLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	FText PortalLabel;

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void ConfigurePortal(FName InTargetLevelName, const FText& InLabel);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void DoOpenLevel();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UBoxComponent> TriggerVolume;

	FTimerHandle TravelTimerHandle;
	bool bPendingTravel = false;
};
