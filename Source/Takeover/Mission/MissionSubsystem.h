#pragma once

#include "CoreMinimal.h"
#include "Chop/TakeoverChopSession.h"
#include "Mission/MissionFlow.h"
#include "Subsystems/WorldSubsystem.h"
#include "MissionSubsystem.generated.h"

class ATakeoverArena;
class ATakeoverPlayerController;
class ATakeoverVehiclePawn;
class UMissionConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMissionPhaseChangedSignature, EMissionPhase, NewPhase);

/** What the phases hand to each other during one loop (PLAN.md section 4) */
struct FMissionLoopState
{
	TWeakObjectPtr<ATakeoverVehiclePawn> Vehicle;
	float TakeoverScore = 0.f;
	int32 RepEarned = 0;
	int32 ChopPayout = 0;
};

/**
 * Runs the mission loop for the world. A tickable world subsystem, so phases tick without a host actor (ADR 0004).
 * POC: phases are handled in a switch here instead of UMissionPhase objects; see
 * docs/superpowers/specs/2026-11-01-core-loop-poc-design.md for when that changes.
 */
UCLASS()
class TAKEOVER_API UMissionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	UFUNCTION(BlueprintPure, Category = "Takeover|Mission")
	EMissionPhase GetPhase() const { return Flow.GetPhase(); }

	/** One line telling the player what to do now */
	FText GetObjectiveText() const;

	float GetTakeoverSecondsRemaining() const { return TakeoverSecondsRemaining; }
	bool IsTakeoverTimerRunning() const { return bTakeoverTimerRunning; }
	const FMissionLoopState& GetLoopState() const { return LoopState; }

	const FTakeoverChopSession& GetChopSession() const { return ChopSession; }
	bool StartChopRemoval(int32 PartIndex);

	/** Sells what has come off, closes the chop menu, and ends the Chop phase */
	void FinishChop();

	/** Debug: completes the current phase as if its condition were met */
	void SkipPhase();

	/** Debug: reloads the level, which starts a new loop. Money and rep survive in the game instance. */
	void RestartLoop();

	UPROPERTY(BlueprintAssignable, Category = "Takeover|Mission")
	FMissionPhaseChangedSignature OnPhaseChanged;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void StartLoop();
	void CompletePhase(EMissionPhase Phase);
	void EnterPhase(EMissionPhase Phase);

	void HideKeys();
	void EndTakeover();
	void FinishLoop();

	const UMissionConfig* GetConfig() const;
	ATakeoverPlayerController* GetPlayerController() const;
	ATakeoverVehiclePawn* GetPlayerVehicle() const;
	ATakeoverArena* FindArena() const;

	FMissionFlow Flow;
	FMissionLoopState LoopState;
	FTakeoverChopSession ChopSession;

	/** The loop starts on the first tick with a player pawn, after every actor has begun play */
	bool bStartPending = false;

	bool bTakeoverTimerRunning = false;
	float TakeoverSecondsRemaining = 0.f;
};
