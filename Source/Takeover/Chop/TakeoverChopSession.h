#pragma once

#include "CoreMinimal.h"

/** A part on the car being chopped */
struct FTakeoverChopPart
{
	FName Id;
	FText DisplayName;
	int32 Value = 0;
	float RemovalSeconds = 0.f;
	bool bRemoved = false;
};

/**
 * Chop shop state for one car, with no world access, so tests drive it directly.
 * One part comes off at a time; each takes its removal time and then pays its value.
 * The removal timing minigame (PLAN.md 5.4) is not in the POC.
 */
struct TAKEOVER_API FTakeoverChopSession
{
	void Reset(TArray<FTakeoverChopPart> InParts);

	/** False if another part is coming off, the part is gone, or the index is out of range */
	bool StartRemoval(int32 PartIndex);

	/** Advances the removal in progress. Returns the value earned on the tick the part comes off, otherwise 0. */
	int32 Tick(float DeltaSeconds);

	const TArray<FTakeoverChopPart>& GetParts() const { return Parts; }
	int32 GetPayout() const { return Payout; }
	bool IsRemoving() const { return RemovingIndex != INDEX_NONE; }
	int32 GetRemovingIndex() const { return RemovingIndex; }

	/** 0 to 1 for the part coming off, 0 when idle */
	float GetRemovalProgress() const;

	bool AllRemoved() const;

private:
	TArray<FTakeoverChopPart> Parts;
	int32 RemovingIndex = INDEX_NONE;
	float RemovalElapsed = 0.f;
	int32 Payout = 0;
};
