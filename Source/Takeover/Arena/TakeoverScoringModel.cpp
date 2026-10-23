#include "Arena/TakeoverScoringModel.h"

namespace
{
	constexpr float KmhPerCmPerSec = 0.036f;

	// Below this the velocity direction is noise
	constexpr double MinDirectionSpeedCmPerSec = 1.0;
}

float FTakeoverScoringModel::SlipAngleDeg(const FVector2D& Forward, const FVector2D& Velocity)
{
	const double Speed = Velocity.Size();
	if (Speed < MinDirectionSpeedCmPerSec)
	{
		return 0.f;
	}

	const FVector2D ForwardDir = Forward.GetSafeNormal();
	const double CosAngle = FMath::Clamp(FVector2D::DotProduct(ForwardDir, Velocity / Speed), -1.0, 1.0);
	return static_cast<float>(FMath::RadiansToDegrees(FMath::Acos(CosAngle)));
}

float FTakeoverScoringModel::SpeedKmh(const FVector2D& Velocity)
{
	return static_cast<float>(Velocity.Size()) * KmhPerCmPerSec;
}

ETakeoverScoringState FTakeoverScoringModel::Classify(const FTakeoverScoringParams& Params, const FTakeoverVehicleSample& Sample)
{
	const float Speed = SpeedKmh(Sample.Velocity);

	// A donut also has a high slip angle, so it is checked before drift
	if (FMath::Abs(Sample.YawRateDegPerSec) >= Params.DonutMinYawRateDegPerSec
		&& Speed <= Params.DonutMaxSpeedKmh
		&& Sample.Throttle >= Params.DonutMinThrottle)
	{
		return ETakeoverScoringState::Donut;
	}

	const float Slip = SlipAngleDeg(Sample.Forward, Sample.Velocity);
	if (Speed >= Params.DriftMinSpeedKmh
		&& Slip >= Params.DriftMinSlipAngleDeg
		&& Slip <= Params.DriftMaxSlipAngleDeg)
	{
		return ETakeoverScoringState::Drift;
	}

	if (Speed <= Params.BurnoutMaxSpeedKmh
		&& Sample.Throttle >= Params.BurnoutMinThrottle
		&& Sample.Brake >= Params.BurnoutMinBrake)
	{
		return ETakeoverScoringState::Burnout;
	}

	return ETakeoverScoringState::None;
}

float FTakeoverScoringModel::ProximityMultiplier(const FTakeoverScoringParams& Params, float DistanceToCrowdCm)
{
	const float Range = Params.ProximityFarDistanceCm - Params.ProximityNearDistanceCm;
	if (Range <= 0.f)
	{
		return DistanceToCrowdCm <= Params.ProximityNearDistanceCm ? Params.ProximityMaxMultiplier : 1.f;
	}

	const float Alpha = FMath::Clamp((Params.ProximityFarDistanceCm - DistanceToCrowdCm) / Range, 0.f, 1.f);
	return FMath::Lerp(1.f, Params.ProximityMaxMultiplier, Alpha);
}

float FTakeoverScoringModel::BasePointsPerSecond(const FTakeoverScoringParams& Params, ETakeoverScoringState InState, const FTakeoverVehicleSample& Sample)
{
	const float Proximity = ProximityMultiplier(Params, Sample.DistanceToCrowdCm);

	switch (InState)
	{
	case ETakeoverScoringState::Drift:
	{
		const float Slip = FMath::Min(SlipAngleDeg(Sample.Forward, Sample.Velocity), Params.DriftMaxSlipAngleDeg);
		const float ReferenceSpeed = FMath::Max(Params.DriftReferenceSpeedKmh, 1.f);
		const float SpeedFactor = FMath::Clamp(SpeedKmh(Sample.Velocity) / ReferenceSpeed, 0.f, Params.DriftMaxSpeedFactor);
		return FMath::Max(Slip - Params.DriftMinSlipAngleDeg, 0.f) * Params.DriftPointsPerDegree * SpeedFactor * Proximity;
	}
	case ETakeoverScoringState::Donut:
		return Params.DonutPointsPerSecond * Proximity;
	case ETakeoverScoringState::Burnout:
		return Params.BurnoutPointsPerSecond * Proximity;
	default:
		return 0.f;
	}
}

FTakeoverScoringTickResult FTakeoverScoringModel::Tick(const FTakeoverScoringParams& Params, const FTakeoverVehicleSample& Sample, float DeltaSeconds)
{
	FTakeoverScoringTickResult Result;
	Result.SlipAngleDeg = SlipAngleDeg(Sample.Forward, Sample.Velocity);
	Result.SpeedKmh = SpeedKmh(Sample.Velocity);
	Result.ProximityMultiplier = ProximityMultiplier(Params, Sample.DistanceToCrowdCm);
	Result.bHitCrowd = Sample.bHitCrowd;

	if (Sample.bHitCrowd || Sample.bCollided)
	{
		Result.bComboBroken = BreakCombo();
	}

	State = Classify(Params, Sample);
	Result.State = State;

	if (State == ETakeoverScoringState::None)
	{
		IdleSeconds += DeltaSeconds;
		if (IdleSeconds >= Params.ComboGraceSeconds && BreakCombo())
		{
			Result.bComboBroken = true;
		}
		return Result;
	}

	IdleSeconds = 0.f;
	ScoringSeconds += DeltaSeconds;
	while (ScoringSeconds >= Params.ComboStepSeconds && Combo < Params.ComboMax)
	{
		ScoringSeconds -= Params.ComboStepSeconds;
		++Combo;
		Result.bComboIncreased = true;
	}
	if (Combo >= Params.ComboMax)
	{
		ScoringSeconds = FMath::Min(ScoringSeconds, Params.ComboStepSeconds);
	}

	Result.Points = BasePointsPerSecond(Params, State, Sample) * Combo * DeltaSeconds;
	Score += Result.Points;
	return Result;
}

void FTakeoverScoringModel::Reset()
{
	*this = FTakeoverScoringModel();
}

bool FTakeoverScoringModel::BreakCombo()
{
	const bool bHadCombo = Combo > 1 || ScoringSeconds > 0.f;
	Combo = 1;
	ScoringSeconds = 0.f;
	IdleSeconds = 0.f;
	return bHadCombo;
}
