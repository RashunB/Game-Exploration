#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"

#if WITH_DEV_AUTOMATION_TESTS

// Proves the test pipeline works end to end: build, discovery, headless run
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTakeoverSmokeTest, "Takeover.Smoke.ModuleLoaded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTakeoverSmokeTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Takeover module is loaded"), FModuleManager::Get().IsModuleLoaded(TEXT("Takeover")));
	return true;
}

#endif
