#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Mission/MissionTypes.h"
#include "MissionConfig.generated.h"

class UDataTable;

/** Loop order and loop tunables. Assigned on the game mode Blueprint. */
UCLASS(BlueprintType)
class TAKEOVER_API UMissionConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Getaway has no police yet: it ends when the car enters the crowd ring. Dump is left out until M4. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loop")
	TArray<EMissionPhase> PhaseOrder = { EMissionPhase::BreakIn, EMissionPhase::Getaway, EMissionPhase::Takeover, EMissionPhase::Chop };

	/** Seed for choosing the key spot. 0 picks a new spot every loop. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Break-in")
	int32 KeySeed = 0;

	/** Counted from the moment the car enters the crowd ring. The M5 drift event replaces this. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Takeover", meta = (ClampMin = "1"))
	float TakeoverDurationSeconds = 90.f;

	/** Rep earned per takeover point, rounded down */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Takeover", meta = (ClampMin = "0"))
	float RepPerScorePoint = 0.01f;

	/** Rows are FTakeoverChopPartRow */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chop", meta = (RequiredAssetDataTags = "RowStructure=/Script/Takeover.TakeoverChopPartRow"))
	TObjectPtr<UDataTable> ChopParts;
};
