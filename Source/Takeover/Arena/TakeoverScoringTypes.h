#pragma once

#include "CoreMinimal.h"
#include "TakeoverScoringTypes.generated.h"

UENUM(BlueprintType)
enum class ETakeoverScoringState : uint8
{
	None,
	Drift,
	Donut,
	Burnout
};

/** Tunables for FTakeoverScoringModel. Edited on a UTakeoverScoringConfig asset. */
USTRUCT(BlueprintType)
struct TAKEOVER_API FTakeoverScoringParams
{
	GENERATED_BODY()

	/** Below this speed a high slip angle is noise, not a drift */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drift", meta = (ClampMin = "0"))
	float DriftMinSpeedKmh = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drift", meta = (ClampMin = "0", ClampMax = "180"))
	float DriftMinSlipAngleDeg = 15.f;

	/** Above this the car is spinning out or reversing, not drifting */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drift", meta = (ClampMin = "0", ClampMax = "180"))
	float DriftMaxSlipAngleDeg = 100.f;

	/** Speed at which the drift speed factor is 1 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drift", meta = (ClampMin = "1"))
	float DriftReferenceSpeedKmh = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drift", meta = (ClampMin = "0"))
	float DriftMaxSpeedFactor = 3.f;

	/** Points per second for each degree of slip above the minimum, before speed, proximity, and combo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drift", meta = (ClampMin = "0"))
	float DriftPointsPerDegree = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Donut", meta = (ClampMin = "0"))
	float DonutMinYawRateDegPerSec = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Donut", meta = (ClampMin = "0"))
	float DonutMaxSpeedKmh = 30.f;

	/** Keeps a spin-out after a crash from scoring as a donut */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Donut", meta = (ClampMin = "0", ClampMax = "1"))
	float DonutMinThrottle = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Donut", meta = (ClampMin = "0"))
	float DonutPointsPerSecond = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Burnout", meta = (ClampMin = "0"))
	float BurnoutMaxSpeedKmh = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Burnout", meta = (ClampMin = "0", ClampMax = "1"))
	float BurnoutMinThrottle = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Burnout", meta = (ClampMin = "0", ClampMax = "1"))
	float BurnoutMinBrake = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Burnout", meta = (ClampMin = "0"))
	float BurnoutPointsPerSecond = 20.f;

	/** Distance to the crowd line at or beyond which the proximity multiplier is 1 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Proximity", meta = (ClampMin = "0"))
	float ProximityFarDistanceCm = 1500.f;

	/** Distance to the crowd line at or within which the proximity multiplier is at its maximum */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Proximity", meta = (ClampMin = "0"))
	float ProximityNearDistanceCm = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Proximity", meta = (ClampMin = "1"))
	float ProximityMaxMultiplier = 3.f;

	/** Seconds of continuous scoring per combo step */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.1"))
	float ComboStepSeconds = 2.f;

	/** Seconds without scoring before the combo breaks */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0"))
	float ComboGraceSeconds = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "1"))
	int32 ComboMax = 10;
};
