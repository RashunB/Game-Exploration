#include "Chop/TakeoverChopSession.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace TakeoverChopTest
{
	FTakeoverChopPart Part(const TCHAR* Id, int32 Value, float RemovalSeconds)
	{
		FTakeoverChopPart Result;
		Result.Id = FName(Id);
		Result.DisplayName = FText::FromString(Id);
		Result.Value = Value;
		Result.RemovalSeconds = RemovalSeconds;
		return Result;
	}
}

BEGIN_DEFINE_SPEC(FTakeoverChopSessionSpec, "Takeover.Chop.Session", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	FTakeoverChopSession Session;
END_DEFINE_SPEC(FTakeoverChopSessionSpec)

void FTakeoverChopSessionSpec::Define()
{
	using namespace TakeoverChopTest;

	BeforeEach([this]()
	{
		Session.Reset({ Part(TEXT("Wheels"), 400, 2.f), Part(TEXT("Hood"), 150, 1.f), Part(TEXT("Seats"), 250, 0.f) });
	});

	It(TEXT("pays a part's value when its removal time has passed"), [this]()
	{
		TestTrue(TEXT("Started"), Session.StartRemoval(0));
		TestEqual(TEXT("Not done at 1 s"), Session.Tick(1.f), 0);
		TestEqual(TEXT("Half way"), Session.GetRemovalProgress(), 0.5f, 0.001f);
		TestEqual(TEXT("Done at 2 s"), Session.Tick(1.f), 400);
		TestTrue(TEXT("Removed"), Session.GetParts()[0].bRemoved);
		TestEqual(TEXT("Payout"), Session.GetPayout(), 400);
		TestFalse(TEXT("Idle again"), Session.IsRemoving());
	});

	It(TEXT("removes one part at a time"), [this]()
	{
		Session.StartRemoval(0);
		TestFalse(TEXT("Second start refused"), Session.StartRemoval(1));
		TestEqual(TEXT("Still the first part"), Session.GetRemovingIndex(), 0);
	});

	It(TEXT("refuses a part that is already off"), [this]()
	{
		Session.StartRemoval(1);
		Session.Tick(1.f);
		TestFalse(TEXT("Again"), Session.StartRemoval(1));
		TestEqual(TEXT("Paid once"), Session.GetPayout(), 150);
	});

	It(TEXT("refuses an index out of range"), [this]()
	{
		TestFalse(TEXT("Negative"), Session.StartRemoval(-1));
		TestFalse(TEXT("Past the end"), Session.StartRemoval(3));
	});

	It(TEXT("removes a zero-second part on the next tick"), [this]()
	{
		Session.StartRemoval(2);
		TestEqual(TEXT("Progress"), Session.GetRemovalProgress(), 1.f, 0.001f);
		TestEqual(TEXT("Value"), Session.Tick(0.f), 250);
	});

	It(TEXT("earns nothing when idle"), [this]()
	{
		TestEqual(TEXT("Tick"), Session.Tick(5.f), 0);
		TestEqual(TEXT("Progress"), Session.GetRemovalProgress(), 0.f, 0.001f);
	});

	It(TEXT("totals every part once all are off"), [this]()
	{
		for (int32 Index = 0; Index < 3; ++Index)
		{
			Session.StartRemoval(Index);
			Session.Tick(10.f);
		}
		TestTrue(TEXT("All removed"), Session.AllRemoved());
		TestEqual(TEXT("Payout"), Session.GetPayout(), 800);
	});

	It(TEXT("treats an empty car as fully stripped"), [this]()
	{
		Session.Reset({});
		TestTrue(TEXT("All removed"), Session.AllRemoved());
		TestEqual(TEXT("Payout"), Session.GetPayout(), 0);
	});
}

#endif
