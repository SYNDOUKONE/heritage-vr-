#pragma once

#include "CoreMinimal.h"
#include "ArtifactInfo.generated.h"

/** Fiche culturelle d'un objet expose dans le musee virtuel. */
USTRUCT(BlueprintType)
struct FArtifactInfo
{
	GENERATED_BODY()

	/** Identifiant unique, ex. "baoule_masque_goli". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
	FName ArtifactId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
	FText TitleFr;

	/** Denomination dans la langue d'origine, si elle existe. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
	FText TitleLocal;

	/** Culture d'attache, ex. "Baoule", "Senoufo", "Dan/Yacouba". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
	FText Culture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
	FText Region;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact", meta = (MultiLine = true))
	FText DescriptionFr;

	/** Soft path du USoundBase de narration en francais. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact|Audio")
	FSoftObjectPath NarrationAudioFr;

	/** Soft path du USoundBase de narration dans la langue locale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact|Audio")
	FSoftObjectPath NarrationAudioLocal;
};
