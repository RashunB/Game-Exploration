#include "Mission/MissionFlow.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FMissionFlowSpec, "Takeover.Mission.Flow", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	FMissionFlow Flow;
	TArray<EMissionPhase> PocOrder;
END_DEFINE_SPEC(FMissionFlowSpec)

void FMissionFlowSpec::Define()
{
	BeforeEach([this]()
	{
		Flow = FMissionFlow();
		PocOrder = { EMissionPhase::BreakIn, EMissionPhase::Getaway, EMissionPhase::Takeover, EMissionPhase::Chop };
	});

	It(TEXT("is idle before it starts"), [this]()
	{
		TestTrue(TEXT("Idle"), Flow.GetPhase() == EMissionPhase::Idle);
		TestFalse(TEXT("Completing while idle"), Flow.CompletePhase(EMissionPhase::Idle));
	});

	It(TEXT("starts at the first phase in the order"), [this]()
	{
		Flow.Start(PocOrder);
		TestTrue(TEXT("BreakIn"), Flow.GetPhase() == EMissionPhase::BreakIn);
	});

	It(TEXT("walks the order and ends Complete"), [this]()
	{
		Flow.Start(PocOrder);
		for (const EMissionPhase Expected : PocOrder)
		{
			TestTrue(TEXT("Current phase"), Flow.GetPhase() == Expected);
			TestTrue(TEXT("Completed"), Flow.CompletePhase(Expected));
		}
		TestTrue(TEXT("Complete"), Flow.GetPhase() == EMissionPhase::Complete);
		TestTrue(TEXT("Finished"), Flow.IsFinished());
	});

	It(TEXT("ignores a completion for a phase that is not current"), [this]()
	{
		Flow.Start(PocOrder);
		TestFalse(TEXT("Stale completion"), Flow.CompletePhase(EMissionPhase::Takeover));
		TestTrue(TEXT("Still BreakIn"), Flow.GetPhase() == EMissionPhase::BreakIn);
	});

	It(TEXT("ignores a duplicate completion"), [this]()
	{
		Flow.Start(PocOrder);
		Flow.CompletePhase(EMissionPhase::BreakIn);
		TestFalse(TEXT("Duplicate"), Flow.CompletePhase(EMissionPhase::BreakIn));
		TestTrue(TEXT("Getaway"), Flow.GetPhase() == EMissionPhase::Getaway);
	});

	It(TEXT("drops non-playable entries from the order"), [this]()
	{
		Flow.Start({ EMissionPhase::Idle, EMissionPhase::Chop, EMissionPhase::Complete, EMissionPhase::Failed });
		TestTrue(TEXT("Chop"), Flow.GetPhase() == EMissionPhase::Chop);
		Flow.CompletePhase(EMissionPhase::Chop);
		TestTrue(TEXT("Complete"), Flow.GetPhase() == EMissionPhase::Complete);
	});

	It(TEXT("completes at once with an empty order"), [this]()
	{
		Flow.Start({});
		TestTrue(TEXT("Complete"), Flow.GetPhase() == EMissionPhase::Complete);
	});

	It(TEXT("fails from a playable phase and stays failed"), [this]()
	{
		Flow.Start(PocOrder);
		Flow.Fail();
		TestTrue(TEXT("Failed"), Flow.GetPhase() == EMissionPhase::Failed);
		TestFalse(TEXT("No completion after failure"), Flow.CompletePhase(EMissionPhase::BreakIn));
	});

	It(TEXT("does not fail once complete"), [this]()
	{
		Flow.Start({ EMissionPhase::Chop });
		Flow.CompletePhase(EMissionPhase::Chop);
		Flow.Fail();
		TestTrue(TEXT("Complete"), Flow.GetPhase() == EMissionPhase::Complete);
	});

	It(TEXT("restarts cleanly"), [this]()
	{
		Flow.Start(PocOrder);
		Flow.Fail();
		Flow.Start(PocOrder);
		TestTrue(TEXT("BreakIn again"), Flow.GetPhase() == EMissionPhase::BreakIn);
	});
}

#endif
