#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MuseumStateSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVillageVisited, FName, VillageId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArtifactProgressExamined, FName, ArtifactId);

/** Progression du visiteur : villages visites et artefacts examines. */
UCLASS(Config = Game)
class HERITAGEVR_API UMuseumStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Museum|Progress")
	void MarkVillageVisited(FName VillageId);

	UFUNCTION(BlueprintCallable, Category = "Museum|Progress")
	void MarkArtifactExamined(FName ArtifactId);

	UFUNCTION(BlueprintCallable, Category = "Museum|Progress", meta = (CompactNodeTitle = "Visited?"))
	bool IsVillageVisited(FName VillageId) const;

	UFUNCTION(BlueprintCallable, Category = "Museum|Progress", meta = (CompactNodeTitle = "Examined?"))
	bool IsArtifactExamined(FName ArtifactId) const;

	UFUNCTION(BlueprintCallable, Category = "Museum|Progress")
	int32 GetVisitedVillageCount() const;

	UFUNCTION(BlueprintCallable, Category = "Museum|Progress")
	int32 GetExaminedArtifactCount() const;

	UFUNCTION(BlueprintCallable, Category = "Museum|Progress")
	void GetVisitedVillages(TArray<FName>& OutVillages) const;

	UPROPERTY(BlueprintAssignable, Category = "Museum|Progress")
	FOnVillageVisited OnVillageVisited;

	UPROPERTY(BlueprintAssignable, Category = "Museum|Progress")
	FOnArtifactProgressExamined OnArtifactExamined;

private:
	UPROPERTY(VisibleAnywhere, Category = "Museum|Progress")
	TSet<FName> VisitedVillages;

	UPROPERTY(VisibleAnywhere, Category = "Museum|Progress")
	TSet<FName> ExaminedArtifacts;
};
