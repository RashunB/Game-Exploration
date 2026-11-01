#include "Mission/MissionFlow.h"

bool FMissionFlow::IsPlayable(EMissionPhase InPhase)
{
	return InPhase != EMissionPhase::Idle && InPhase != EMissionPhase::Complete && InPhase != EMissionPhase::Failed;
}

void FMissionFlow::Start(const TArray<EMissionPhase>& InOrder)
{
	Order.Reset();
	for (const EMissionPhase Entry : InOrder)
	{
		if (IsPlayable(Entry))
		{
			Order.Add(Entry);
		}
	}

	Index = 0;
	Phase = Order.IsEmpty() ? EMissionPhase::Complete : Order[0];
}

bool FMissionFlow::CompletePhase(EMissionPhase InPhase)
{
	if (IsFinished() || Phase == EMissionPhase::Idle || InPhase != Phase)
	{
		return false;
	}

	++Index;
	Phase = Order.IsValidIndex(Index) ? Order[Index] : EMissionPhase::Complete;
	return true;
}

void FMissionFlow::Fail()
{
	if (Phase != EMissionPhase::Idle && !IsFinished())
	{
		Phase = EMissionPhase::Failed;
	}
}
