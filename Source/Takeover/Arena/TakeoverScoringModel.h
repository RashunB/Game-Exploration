#pragma once

#include "CoreMinimal.h"
#include "Arena/TakeoverScoringTypes.h"

/** One tick of vehicle state, projected onto the ground plane */
struct FTakeoverVehicleSample
{
	FVector2D Forward = FVector2D(1.0, 0.0);

	/** cm/s */
	FVector2D Velocity = FVector2D::ZeroVector;

	float YawRateDegPerSec = 0.f;
	float Throttle = 0.f;
	float Brake = 0.f;

	/** Stays at max when there is no arena, which gives a proximity multiplier of 1 */
	float DistanceToCrowdCm = TNumericLimits<float>::Max();

	bool bCollided = false;
	bool bHitCrowd = false;
};

struct FTakeoverScoringTickResult
{
	ETakeoverScoringState State = ETakeoverScoringState::None;
	float Points = 0.f;
	float SlipAngleDeg = 0.f;
	float SpeedKmh = 0.f;
	float ProximityMultiplier = 1.f;
	bool bComboIncreased = false;
	bool bComboBroken = false;
	bool bHitCrowd = false;
};

/**
 * Takeover scoring math with no world access, so automation tests can drive it directly.
 * UTakeoverScoringComponent feeds it one sample per tick.
 */
struct TAKEOVER_API FTakeoverScoringModel
{
	/** Angle between the car's heading and its velocity, 0 to 180 */
	static float SlipAngleDeg(const FVector2D& Forward, const FVector2D& Velocity);

	static float SpeedKmh(const FVector2D& Velocity);

	/** Exactly one state per tick, checked in priority order: Donut, Drift, Burnout, None */
	static ETakeoverScoringState Classify(const FTakeoverScoringParams& Params, const FTakeoverVehicleSample& Sample);

	static float ProximityMultiplier(const FTakeoverScoringParams& Params, float DistanceToCrowdCm);

	/** Points per second for a state, before the combo multiplier */
	static float BasePointsPerSecond(const FTakeoverScoringParams& Params, ETakeoverScoringState InState, const FTakeoverVehicleSample& Sample);

	FTakeoverScoringTickResult Tick(const FTakeoverScoringParams& Params, const FTakeoverVehicleSample& Sample, float DeltaSeconds);

	void Reset();

	float GetScore() const { return Score; }
	int32 GetCombo() const { return Combo; }
	ETakeoverScoringState GetState() const { return State; }

private:
	/** Returns true if there was a combo to break */
	bool BreakCombo();

	float Score = 0.f;
	int32 Combo = 1;
	float ScoringSeconds = 0.f;
	float IdleSeconds = 0.f;
	ETakeoverScoringState State = ETakeoverScoringState::None;
};
