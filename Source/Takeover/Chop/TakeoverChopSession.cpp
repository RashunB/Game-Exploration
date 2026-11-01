#include "Chop/TakeoverChopSession.h"

void FTakeoverChopSession::Reset(TArray<FTakeoverChopPart> InParts)
{
	*this = FTakeoverChopSession();
	Parts = MoveTemp(InParts);
}

bool FTakeoverChopSession::StartRemoval(int32 PartIndex)
{
	if (IsRemoving() || !Parts.IsValidIndex(PartIndex) || Parts[PartIndex].bRemoved)
	{
		return false;
	}

	RemovingIndex = PartIndex;
	RemovalElapsed = 0.f;
	return true;
}

int32 FTakeoverChopSession::Tick(float DeltaSeconds)
{
	if (!IsRemoving())
	{
		return 0;
	}

	RemovalElapsed += DeltaSeconds;
	FTakeoverChopPart& Part = Parts[RemovingIndex];
	if (RemovalElapsed < Part.RemovalSeconds)
	{
		return 0;
	}

	Part.bRemoved = true;
	Payout += Part.Value;
	RemovingIndex = INDEX_NONE;
	RemovalElapsed = 0.f;
	return Part.Value;
}

float FTakeoverChopSession::GetRemovalProgress() const
{
	if (!IsRemoving())
	{
		return 0.f;
	}

	const float Duration = Parts[RemovingIndex].RemovalSeconds;
	return Duration > 0.f ? FMath::Clamp(RemovalElapsed / Duration, 0.f, 1.f) : 1.f;
}

bool FTakeoverChopSession::AllRemoved() const
{
	for (const FTakeoverChopPart& Part : Parts)
	{
		if (!Part.bRemoved)
		{
			return false;
		}
	}
	return true;
}
