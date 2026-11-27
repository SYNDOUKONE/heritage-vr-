#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VillageZone.generated.h"

class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVillageEntered, class AVillageZone*, Zone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVillageExited, class AVillageZone*, Zone);

/** Volume declencheur delimitant un village historique reconstitue. */
UCLASS()
class HERITAGEVR_API AVillageZone : public AActor
{
	GENERATED_BODY()

public:
	AVillageZone();

	UPROPERTY(BlueprintAssignable, Category = "Village")
	FOnVillageEntered OnVillageEntered;

	UPROPERTY(BlueprintAssignable, Category = "Village")
	FOnVillageExited OnVillageExited;

	UFUNCTION(BlueprintCallable, Category = "Village")
	void ConfigureZone(const FName& InVillageId, const FText& InDisplayName, const FText& InDescriptionFr);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	/** Zone du village, definie dans le niveau a l'echelle reelle du site. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Village")
	TObjectPtr<UBoxComponent> ZoneVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Village")
	FName VillageId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Village")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Village", meta = (MultiLine = true))
	FText DescriptionFr;

	/** Soft path du son d'ambiance du village (foret, riviere, cour du village...). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Village|Audio")
	FSoftObjectPath AmbienceSound;
};
