#include "Arena/TakeoverRingGeometry.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FTakeoverRingGeometrySpec, "Takeover.Arena.Ring", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	TArray<FVector2D> Square;
END_DEFINE_SPEC(FTakeoverRingGeometrySpec)

void FTakeoverRingGeometrySpec::Define()
{
	BeforeEach([this]()
	{
		Square = { FVector2D(-100.0, -100.0), FVector2D(100.0, -100.0), FVector2D(100.0, 100.0), FVector2D(-100.0, 100.0) };
	});

	It(TEXT("finds the center inside"), [this]()
	{
		TestTrue(TEXT("Center"), FTakeoverRingGeometry::IsInside(Square, FVector2D::ZeroVector));
	});

	It(TEXT("finds points beyond each side outside"), [this]()
	{
		TestFalse(TEXT("East"), FTakeoverRingGeometry::IsInside(Square, FVector2D(150.0, 0.0)));
		TestFalse(TEXT("West"), FTakeoverRingGeometry::IsInside(Square, FVector2D(-150.0, 0.0)));
		TestFalse(TEXT("North"), FTakeoverRingGeometry::IsInside(Square, FVector2D(0.0, 150.0)));
		TestFalse(TEXT("South"), FTakeoverRingGeometry::IsInside(Square, FVector2D(0.0, -150.0)));
	});

	It(TEXT("handles a concave ring"), [this]()
	{
		// A U shape: the notch between the arms is outside
		const TArray<FVector2D> U = { FVector2D(0.0, 0.0), FVector2D(300.0, 0.0), FVector2D(300.0, 300.0), FVector2D(200.0, 300.0),
			FVector2D(200.0, 100.0), FVector2D(100.0, 100.0), FVector2D(100.0, 300.0), FVector2D(0.0, 300.0) };
		TestTrue(TEXT("Left arm"), FTakeoverRingGeometry::IsInside(U, FVector2D(50.0, 200.0)));
		TestFalse(TEXT("Notch"), FTakeoverRingGeometry::IsInside(U, FVector2D(150.0, 200.0)));
		TestTrue(TEXT("Base"), FTakeoverRingGeometry::IsInside(U, FVector2D(150.0, 50.0)));
	});

	It(TEXT("is never inside fewer than three points"), [this]()
	{
		TestFalse(TEXT("Empty"), FTakeoverRingGeometry::IsInside({}, FVector2D::ZeroVector));
		TestFalse(TEXT("Line"), FTakeoverRingGeometry::IsInside({ FVector2D(-1.0, 0.0), FVector2D(1.0, 0.0) }, FVector2D::ZeroVector));
	});
}

#endif
