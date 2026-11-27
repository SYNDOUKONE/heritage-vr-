// Copyright (c) 2026 Heritage VR contributors. All rights reserved.

#include "Museum/MuseumStateSubsystem.h"

void UMuseumStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("MuseumStateSubsystem initialised"));
}

void UMuseumStateSubsystem::MarkVillageVisited(FName VillageId)
{
	if (VillageId.IsNone() || VisitedVillages.Contains(VillageId))
	{
		return;
	}

	VisitedVillages.Add(VillageId);
	OnVillageVisited.Broadcast(VillageId);
}

void UMuseumStateSubsystem::MarkArtifactExamined(FName ArtifactId)
{
	if (ArtifactId.IsNone() || ExaminedArtifacts.Contains(ArtifactId))
	{
		return;
	}

	ExaminedArtifacts.Add(ArtifactId);
	OnArtifactExamined.Broadcast(ArtifactId);
}

bool UMuseumStateSubsystem::IsVillageVisited(FName VillageId) const
{
	return VisitedVillages.Contains(VillageId);
}

bool UMuseumStateSubsystem::IsArtifactExamined(FName ArtifactId) const
{
	return ExaminedArtifacts.Contains(ArtifactId);
}

int32 UMuseumStateSubsystem::GetVisitedVillageCount() const
{
	return VisitedVillages.Num();
}

int32 UMuseumStateSubsystem::GetExaminedArtifactCount() const
{
	return ExaminedArtifacts.Num();
}

void UMuseumStateSubsystem::GetVisitedVillages(TArray<FName>& OutVillages) const
{
	VisitedVillages.GetKeys(OutVillages);
}
