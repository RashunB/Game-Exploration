#pragma once

#include "CoreMinimal.h"
#include "Arena/TakeoverScoringTypes.h"
#include "Engine/DataAsset.h"
#include "TakeoverScoringConfig.generated.h"

/** All takeover scoring tunables. One asset per scoring setup, assigned on the vehicle's scoring component. */
UCLASS(BlueprintType)
class TAKEOVER_API UTakeoverScoringConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoring")
	FTakeoverScoringParams Params;

	/** Hits softer than this don't break the combo. Units are kg cm/s. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision", meta = (ClampMin = "0"))
	float MinCollisionImpulse = 30000.f;

	/** Hits with an impact normal steeper than this are ground contact (landings, bottoming out) and ignored */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision", meta = (ClampMin = "0", ClampMax = "1"))
	float GroundNormalMinZ = 0.7f;

	/** Sustained contact with the crowd fires hit events every frame; this limits it to one crowd hit per window */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision", meta = (ClampMin = "0"))
	float CrowdHitCooldownSeconds = 1.f;
};
