#pragma once

#include "CoreMinimal.h"
#include "Mission/MissionTypes.h"

/**
 * Phase order for one loop, with no world access, so tests drive it directly.
 * UMissionSubsystem owns one and tells it when the current phase is done.
 */
struct TAKEOVER_API FMissionFlow
{
	/** Playable phases only; Idle, Complete, and Failed in the order are ignored. An empty order completes at once. */
	void Start(const TArray<EMissionPhase>& InOrder);

	/** Moves to the next phase if Phase is the current one. Returns false for a stale or duplicate completion. */
	bool CompletePhase(EMissionPhase Phase);

	void Fail();

	EMissionPhase GetPhase() const { return Phase; }
	bool IsFinished() const { return Phase == EMissionPhase::Complete || Phase == EMissionPhase::Failed; }

	static bool IsPlayable(EMissionPhase InPhase);

private:
	TArray<EMissionPhase> Order;
	int32 Index = INDEX_NONE;
	EMissionPhase Phase = EMissionPhase::Idle;
};
