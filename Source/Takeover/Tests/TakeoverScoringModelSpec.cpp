#include "Arena/TakeoverScoringModel.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace TakeoverScoringTest
{
	// Power-of-two step so accumulated time hits combo boundaries exactly
	constexpr float Step = 0.25f;

	FVector2D VelocityKmh(float Kmh, float HeadingOffsetDeg)
	{
		const float CmPerSec = Kmh / 0.036f;
		const float Radians = FMath::DegreesToRadians(HeadingOffsetDeg);
		return FVector2D(FMath::Cos(Radians), FMath::Sin(Radians)) * CmPerSec;
	}

	/** Car facing +X, moving at Kmh with the given slip angle */
	FTakeoverVehicleSample Moving(float Kmh, float SlipDeg)
	{
		FTakeoverVehicleSample Sample;
		Sample.Velocity = VelocityKmh(Kmh, SlipDeg);
		return Sample;
	}

	FTakeoverVehicleSample Drifting() { return Moving(60.f, 45.f); }
	FTakeoverVehicleSample Cruising() { return Moving(40.f, 0.f); }

	void TickFor(FTakeoverScoringModel& Model, const FTakeoverScoringParams& Params, const FTakeoverVehicleSample& Sample, float Seconds)
	{
		const int32 Ticks = FMath::RoundToInt(Seconds / Step);
		for (int32 Index = 0; Index < Ticks; ++Index)
		{
			Model.Tick(Params, Sample, Step);
		}
	}
}

BEGIN_DEFINE_SPEC(FTakeoverScoringModelSpec, "Takeover.Scoring.Model", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	FTakeoverScoringParams Params;
	FTakeoverScoringModel Model;
END_DEFINE_SPEC(FTakeoverScoringModelSpec)

void FTakeoverScoringModelSpec::Define()
{
	using namespace TakeoverScoringTest;

	BeforeEach([this]()
	{
		Params = FTakeoverScoringParams();
		Model = FTakeoverScoringModel();
	});

	Describe(TEXT("SlipAngleDeg"), [this]()
	{
		It(TEXT("is 0 moving straight ahead"), [this]()
		{
			TestEqual(TEXT("Slip"), FTakeoverScoringModel::SlipAngleDeg(FVector2D(1.0, 0.0), FVector2D(1000.0, 0.0)), 0.f, 0.01f);
		});

		It(TEXT("is 45 moving diagonally"), [this]()
		{
			TestEqual(TEXT("Slip"), FTakeoverScoringModel::SlipAngleDeg(FVector2D(1.0, 0.0), FVector2D(1000.0, 1000.0)), 45.f, 0.01f);
		});

		It(TEXT("is 180 reversing"), [this]()
		{
			TestEqual(TEXT("Slip"), FTakeoverScoringModel::SlipAngleDeg(FVector2D(1.0, 0.0), FVector2D(-1000.0, 0.0)), 180.f, 0.01f);
		});

		It(TEXT("is 0 when stationary"), [this]()
		{
			TestEqual(TEXT("Slip"), FTakeoverScoringModel::SlipAngleDeg(FVector2D(1.0, 0.0), FVector2D::ZeroVector), 0.f, 0.01f);
		});
	});

	Describe(TEXT("Classify"), [this]()
	{
		It(TEXT("detects a drift above min speed and slip"), [this]()
		{
			TestTrue(TEXT("Drift"), FTakeoverScoringModel::Classify(Params, Moving(40.f, 30.f)) == ETakeoverScoringState::Drift);
		});

		It(TEXT("ignores slip below the minimum angle"), [this]()
		{
			TestTrue(TEXT("None"), FTakeoverScoringModel::Classify(Params, Moving(40.f, 10.f)) == ETakeoverScoringState::None);
		});

		It(TEXT("ignores slip below the minimum speed"), [this]()
		{
			TestTrue(TEXT("None"), FTakeoverScoringModel::Classify(Params, Moving(15.f, 30.f)) == ETakeoverScoringState::None);
		});

		It(TEXT("does not count reversing as a drift"), [this]()
		{
			TestTrue(TEXT("None"), FTakeoverScoringModel::Classify(Params, Moving(30.f, 180.f)) == ETakeoverScoringState::None);
		});

		It(TEXT("prefers donut over drift"), [this]()
		{
			FTakeoverVehicleSample Sample = Moving(25.f, 60.f);
			Sample.YawRateDegPerSec = 200.f;
			Sample.Throttle = 1.f;
			TestTrue(TEXT("Donut"), FTakeoverScoringModel::Classify(Params, Sample) == ETakeoverScoringState::Donut);
		});

		It(TEXT("does not count a spin without throttle as a donut"), [this]()
		{
			FTakeoverVehicleSample Sample = Moving(10.f, 60.f);
			Sample.YawRateDegPerSec = 200.f;
			TestTrue(TEXT("None"), FTakeoverScoringModel::Classify(Params, Sample) == ETakeoverScoringState::None);
		});

		It(TEXT("detects a burnout with throttle and brake at low speed"), [this]()
		{
			FTakeoverVehicleSample Sample = Moving(2.f, 0.f);
			Sample.Throttle = 1.f;
			Sample.Brake = 1.f;
			TestTrue(TEXT("Burnout"), FTakeoverScoringModel::Classify(Params, Sample) == ETakeoverScoringState::Burnout);
		});
	});

	Describe(TEXT("ProximityMultiplier"), [this]()
	{
		It(TEXT("is 1 at or beyond the far distance"), [this]()
		{
			TestEqual(TEXT("At far"), FTakeoverScoringModel::ProximityMultiplier(Params, Params.ProximityFarDistanceCm), 1.f, 0.001f);
			TestEqual(TEXT("Beyond far"), FTakeoverScoringModel::ProximityMultiplier(Params, 100000.f), 1.f, 0.001f);
			TestEqual(TEXT("No arena"), FTakeoverScoringModel::ProximityMultiplier(Params, TNumericLimits<float>::Max()), 1.f, 0.001f);
		});

		It(TEXT("is max at or within the near distance"), [this]()
		{
			TestEqual(TEXT("At near"), FTakeoverScoringModel::ProximityMultiplier(Params, Params.ProximityNearDistanceCm), Params.ProximityMaxMultiplier, 0.001f);
			TestEqual(TEXT("Touching"), FTakeoverScoringModel::ProximityMultiplier(Params, 0.f), Params.ProximityMaxMultiplier, 0.001f);
		});

		It(TEXT("interpolates between near and far"), [this]()
		{
			const float Midpoint = 0.5f * (Params.ProximityNearDistanceCm + Params.ProximityFarDistanceCm);
			const float Expected = 0.5f * (1.f + Params.ProximityMaxMultiplier);
			TestEqual(TEXT("Midpoint"), FTakeoverScoringModel::ProximityMultiplier(Params, Midpoint), Expected, 0.001f);
		});
	});

	Describe(TEXT("Points"), [this]()
	{
		It(TEXT("scores drift as slip above minimum times speed factor"), [this]()
		{
			// 45 deg slip at the 60 km/h reference speed, no crowd: (45 - 15) * 1 * 1 = 30 per second
			const FTakeoverScoringTickResult Result = Model.Tick(Params, Drifting(), Step);
			TestEqual(TEXT("Points"), Result.Points, 30.f * Step, 0.05f);
			TestEqual(TEXT("Score"), Model.GetScore(), 30.f * Step, 0.05f);
		});

		It(TEXT("multiplies points by the combo"), [this]()
		{
			const float ComboOnePoints = Model.Tick(Params, Drifting(), Step).Points;
			TickFor(Model, Params, Drifting(), Params.ComboStepSeconds);
			TestEqual(TEXT("Combo"), Model.GetCombo(), 2);
			const float ComboTwoPoints = Model.Tick(Params, Drifting(), Step).Points;
			TestEqual(TEXT("Doubled"), ComboTwoPoints, 2.f * ComboOnePoints, 0.05f);
		});

		It(TEXT("awards nothing when not scoring"), [this]()
		{
			TestEqual(TEXT("Points"), Model.Tick(Params, Cruising(), Step).Points, 0.f, 0.001f);
		});
	});

	Describe(TEXT("Combo"), [this]()
	{
		It(TEXT("increments every step of continuous scoring"), [this]()
		{
			TickFor(Model, Params, Drifting(), Params.ComboStepSeconds);
			TestEqual(TEXT("After one step"), Model.GetCombo(), 2);
			TickFor(Model, Params, Drifting(), Params.ComboStepSeconds);
			TestEqual(TEXT("After two steps"), Model.GetCombo(), 3);
		});

		It(TEXT("caps at the maximum"), [this]()
		{
			Params.ComboMax = 3;
			TickFor(Model, Params, Drifting(), 10.f * Params.ComboStepSeconds);
			TestEqual(TEXT("Capped"), Model.GetCombo(), 3);
		});

		It(TEXT("survives a gap shorter than the grace period"), [this]()
		{
			TickFor(Model, Params, Drifting(), Params.ComboStepSeconds);
			TickFor(Model, Params, Cruising(), 1.f);
			TestEqual(TEXT("Combo"), Model.GetCombo(), 2);
		});

		It(TEXT("breaks after the grace period"), [this]()
		{
			TickFor(Model, Params, Drifting(), Params.ComboStepSeconds);
			TickFor(Model, Params, Cruising(), Params.ComboGraceSeconds);
			TestEqual(TEXT("Combo"), Model.GetCombo(), 1);
		});

		It(TEXT("breaks on collision"), [this]()
		{
			TickFor(Model, Params, Drifting(), Params.ComboStepSeconds);
			FTakeoverVehicleSample Crash = Drifting();
			Crash.bCollided = true;
			const FTakeoverScoringTickResult Result = Model.Tick(Params, Crash, Step);
			TestTrue(TEXT("Reported broken"), Result.bComboBroken);
			TestEqual(TEXT("Combo"), Model.GetCombo(), 1);
		});

		It(TEXT("breaks and reports on a crowd hit"), [this]()
		{
			TickFor(Model, Params, Drifting(), Params.ComboStepSeconds);
			FTakeoverVehicleSample Hit = Drifting();
			Hit.bHitCrowd = true;
			const FTakeoverScoringTickResult Result = Model.Tick(Params, Hit, Step);
			TestTrue(TEXT("Crowd hit reported"), Result.bHitCrowd);
			TestTrue(TEXT("Reported broken"), Result.bComboBroken);
			TestEqual(TEXT("Combo"), Model.GetCombo(), 1);
		});
	});
}

#endif
