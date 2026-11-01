#pragma once

#include "CoreMinimal.h"
#include "MissionTypes.generated.h"

/** Phases of one mission loop, in PLAN.md order. Idle, Complete, and Failed are states, not playable phases. */
UENUM(BlueprintType)
enum class EMissionPhase : uint8
{
	Idle,
	BreakIn,
	Getaway,
	Takeover,
	Chop,
	Dump,
	Complete,
	Failed
};
