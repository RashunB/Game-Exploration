#include "Mission/MissionSubsystem.h"

#include "Arena/TakeoverArena.h"
#include "Arena/TakeoverScoringComponent.h"
#include "Chop/TakeoverChopTypes.h"
#include "Core/TakeoverGameMode.h"
#include "Core/TakeoverProgressSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Mission/MissionConfig.h"
#include "Player/TakeoverCharacter.h"
#include "Player/TakeoverPlayerController.h"
#include "Stealth/TakeoverKeySpot.h"
#include "Takeover.h"
#include "Vehicle/TakeoverVehiclePawn.h"

#define LOCTEXT_NAMESPACE "TakeoverMission"

namespace
{
	UMissionSubsystem* MissionFor(UWorld* World)
	{
		return World ? World->GetSubsystem<UMissionSubsystem>() : nullptr;
	}

	FAutoConsoleCommandWithWorld SkipPhaseCommand(
		TEXT("takeover.Mission.Skip"),
		TEXT("Complete the current mission phase as if its condition were met"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (UMissionSubsystem* Mission = MissionFor(World))
			{
				Mission->SkipPhase();
			}
		}));

	FAutoConsoleCommandWithWorld RestartLoopCommand(
		TEXT("takeover.Mission.Restart"),
		TEXT("Reload the level and start a new loop; money and rep are kept"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (UMissionSubsystem* Mission = MissionFor(World))
			{
				Mission->RestartLoop();
			}
		}));
}

bool UMissionSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UMissionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Other maps and game modes have no loop
	bStartPending = InWorld.GetAuthGameMode<ATakeoverGameMode>() != nullptr;
}

TStatId UMissionSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMissionSubsystem, STATGROUP_Tickables);
}

void UMissionSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bStartPending)
	{
		const ATakeoverPlayerController* Controller = GetPlayerController();
		if (!Controller || !Controller->GetPawn())
		{
			return;
		}
		bStartPending = false;
		StartLoop();
	}

	switch (Flow.GetPhase())
	{
	case EMissionPhase::BreakIn:
		// Done once the player is driving: the car only lets someone in who has the keys
		if (ATakeoverVehiclePawn* Vehicle = GetPlayerVehicle())
		{
			LoopState.Vehicle = Vehicle;
			CompletePhase(EMissionPhase::BreakIn);
		}
		break;

	case EMissionPhase::Getaway:
	{
		const ATakeoverVehiclePawn* Vehicle = GetPlayerVehicle();
		const ATakeoverArena* Arena = FindArena();
		if (!Arena)
		{
			UE_LOG(LogTakeover, Warning, TEXT("No ATakeoverArena in the level, skipping the drive to it"));
			CompletePhase(EMissionPhase::Getaway);
		}
		else if (Vehicle && Arena->IsInsideCrowdLine(Vehicle->GetActorLocation()))
		{
			CompletePhase(EMissionPhase::Getaway);
		}
		break;
	}

	case EMissionPhase::Takeover:
		TakeoverSecondsRemaining = FMath::Max(TakeoverSecondsRemaining - DeltaTime, 0.f);
		if (TakeoverSecondsRemaining <= 0.f)
		{
			CompletePhase(EMissionPhase::Takeover);
		}
		break;

	case EMissionPhase::Chop:
		ChopSession.Tick(DeltaTime);
		break;

	default:
		break;
	}
}

void UMissionSubsystem::StartLoop()
{
	LoopState = FMissionLoopState();
	ChopSession.Reset({});
	bTakeoverTimerRunning = false;
	TakeoverSecondsRemaining = 0.f;

	Flow.Start(GetConfig()->PhaseOrder);
	UE_LOG(LogTakeover, Log, TEXT("Mission loop started"));
	EnterPhase(Flow.GetPhase());
}

void UMissionSubsystem::CompletePhase(EMissionPhase Phase)
{
	if (Phase == EMissionPhase::Takeover && Flow.GetPhase() == EMissionPhase::Takeover)
	{
		EndTakeover();
	}

	if (Flow.CompletePhase(Phase))
	{
		EnterPhase(Flow.GetPhase());
	}
}

void UMissionSubsystem::EnterPhase(EMissionPhase Phase)
{
	UE_LOG(LogTakeover, Log, TEXT("Mission phase: %s"), *UEnum::GetValueAsString(Phase));

	// Broadcast before the phase's own setup, which can complete it at once (Dump) and enter the next one
	OnPhaseChanged.Broadcast(Phase);

	switch (Phase)
	{
	case EMissionPhase::BreakIn:
		HideKeys();
		break;

	case EMissionPhase::Takeover:
		// Only the takeover counts; drifting on the way there scores nothing for the loop
		if (ATakeoverVehiclePawn* Vehicle = GetPlayerVehicle())
		{
			Vehicle->GetScoring()->ResetScore();
		}
		bTakeoverTimerRunning = true;
		TakeoverSecondsRemaining = GetConfig()->TakeoverDurationSeconds;
		break;

	case EMissionPhase::Chop:
	{
		TArray<FTakeoverChopPart> Parts;
		if (const UDataTable* Table = GetConfig()->ChopParts)
		{
			Table->ForeachRow<FTakeoverChopPartRow>(TEXT("Chop"), [&Parts](const FName& RowName, const FTakeoverChopPartRow& Row)
			{
				FTakeoverChopPart& Part = Parts.AddDefaulted_GetRef();
				Part.Id = RowName;
				Part.DisplayName = Row.DisplayName.IsEmpty() ? FText::FromName(RowName) : Row.DisplayName;
				Part.Value = Row.BaseValue;
				Part.RemovalSeconds = Row.RemovalSeconds;
			});
		}
		else
		{
			UE_LOG(LogTakeover, Warning, TEXT("Mission config has no ChopParts table; the car has nothing to sell"));
		}
		ChopSession.Reset(MoveTemp(Parts));

		if (ATakeoverVehiclePawn* Vehicle = GetPlayerVehicle())
		{
			Vehicle->ParkForChop();
		}
		if (ATakeoverPlayerController* Controller = GetPlayerController())
		{
			Controller->OpenChopMenu();
		}
		break;
	}

	case EMissionPhase::Dump:
		// Not built until M4; pass straight through so a config that lists it still finishes
		UE_LOG(LogTakeover, Warning, TEXT("Dump is not implemented yet, skipping it"));
		CompletePhase(EMissionPhase::Dump);
		break;

	case EMissionPhase::Complete:
		FinishLoop();
		break;

	default:
		break;
	}
}

void UMissionSubsystem::HideKeys()
{
	TArray<ATakeoverKeySpot*> Spots;
	for (TActorIterator<ATakeoverKeySpot> It(GetWorld()); It; ++It)
	{
		Spots.Add(*It);
	}

	if (Spots.IsEmpty())
	{
		// Lets the loop run before the greybox house is built
		UE_LOG(LogTakeover, Warning, TEXT("No ATakeoverKeySpot in the level, handing the player the keys"));
		if (ATakeoverPlayerController* Controller = GetPlayerController())
		{
			if (ATakeoverCharacter* Character = Cast<ATakeoverCharacter>(Controller->GetPawn()))
			{
				Character->GiveKeys();
			}
		}
		return;
	}

	const int32 Seed = GetConfig()->KeySeed;
	FRandomStream Random(Seed != 0 ? Seed : FMath::Rand());
	const int32 KeyIndex = Random.RandRange(0, Spots.Num() - 1);
	for (int32 Index = 0; Index < Spots.Num(); ++Index)
	{
		Spots[Index]->ResetSearch(Index == KeyIndex);
	}
	UE_LOG(LogTakeover, Log, TEXT("Keys hidden at %s (%d spots)"), *Spots[KeyIndex]->GetName(), Spots.Num());
}

void UMissionSubsystem::EndTakeover()
{
	bTakeoverTimerRunning = false;

	const ATakeoverVehiclePawn* Vehicle = GetPlayerVehicle();
	LoopState.TakeoverScore = Vehicle ? Vehicle->GetScoring()->GetScore() : 0.f;
	LoopState.RepEarned = FMath::FloorToInt(LoopState.TakeoverScore * GetConfig()->RepPerScorePoint);

	if (UTakeoverProgressSubsystem* Progress = GetWorld()->GetGameInstance()->GetSubsystem<UTakeoverProgressSubsystem>())
	{
		Progress->AddRep(LoopState.RepEarned);
	}
}

bool UMissionSubsystem::StartChopRemoval(int32 PartIndex)
{
	return Flow.GetPhase() == EMissionPhase::Chop && ChopSession.StartRemoval(PartIndex);
}

void UMissionSubsystem::FinishChop()
{
	if (Flow.GetPhase() != EMissionPhase::Chop)
	{
		return;
	}

	// A part still coming off is abandoned, not paid
	LoopState.ChopPayout = ChopSession.GetPayout();
	if (UTakeoverProgressSubsystem* Progress = GetWorld()->GetGameInstance()->GetSubsystem<UTakeoverProgressSubsystem>())
	{
		Progress->AddMoney(LoopState.ChopPayout);
	}
	if (ATakeoverPlayerController* Controller = GetPlayerController())
	{
		Controller->CloseChopMenu();
	}
	CompletePhase(EMissionPhase::Chop);
}

void UMissionSubsystem::FinishLoop()
{
	if (UTakeoverProgressSubsystem* Progress = GetWorld()->GetGameInstance()->GetSubsystem<UTakeoverProgressSubsystem>())
	{
		Progress->RecordLoopCompleted();
	}
	UE_LOG(LogTakeover, Log, TEXT("Loop complete: score %.0f, rep +%d, money +%d"), LoopState.TakeoverScore, LoopState.RepEarned, LoopState.ChopPayout);
}

void UMissionSubsystem::SkipPhase()
{
	const EMissionPhase Phase = Flow.GetPhase();
	if (Phase == EMissionPhase::Chop)
	{
		FinishChop();
		return;
	}
	if (Phase == EMissionPhase::BreakIn)
	{
		LoopState.Vehicle = GetPlayerVehicle();
	}
	UE_LOG(LogTakeover, Log, TEXT("Skipping mission phase %s"), *UEnum::GetValueAsString(Phase));
	CompletePhase(Phase);
}

void UMissionSubsystem::RestartLoop()
{
	UWorld* World = GetWorld();
	UGameplayStatics::OpenLevel(World, FName(*UGameplayStatics::GetCurrentLevelName(World)));
}

FText UMissionSubsystem::GetObjectiveText() const
{
	switch (Flow.GetPhase())
	{
	case EMissionPhase::BreakIn:
	{
		const ATakeoverPlayerController* Controller = GetPlayerController();
		const ATakeoverCharacter* Character = Controller ? Cast<ATakeoverCharacter>(Controller->GetPawn()) : nullptr;
		return Character && Character->HasKeys()
			? LOCTEXT("ObjectiveCar", "You have the keys. Get in the car.")
			: LOCTEXT("ObjectiveKeys", "Search the house for the car keys.");
	}
	case EMissionPhase::Getaway:
		return LOCTEXT("ObjectiveDrive", "Drive to the takeover.");
	case EMissionPhase::Takeover:
		return LOCTEXT("ObjectiveTakeover", "Put on a show for the crowd.");
	case EMissionPhase::Chop:
		return LOCTEXT("ObjectiveChop", "Strip the car and sell the parts.");
	case EMissionPhase::Complete:
		return LOCTEXT("ObjectiveComplete", "Loop complete. takeover.Mission.Restart starts another.");
	case EMissionPhase::Failed:
		return LOCTEXT("ObjectiveFailed", "Loop failed.");
	default:
		return FText::GetEmpty();
	}
}

const UMissionConfig* UMissionSubsystem::GetConfig() const
{
	const ATakeoverGameMode* GameMode = GetWorld()->GetAuthGameMode<ATakeoverGameMode>();
	const UMissionConfig* Config = GameMode ? GameMode->GetMissionConfig() : nullptr;
	return Config ? Config : GetDefault<UMissionConfig>();
}

ATakeoverPlayerController* UMissionSubsystem::GetPlayerController() const
{
	return Cast<ATakeoverPlayerController>(GetWorld()->GetFirstPlayerController());
}

ATakeoverVehiclePawn* UMissionSubsystem::GetPlayerVehicle() const
{
	const ATakeoverPlayerController* Controller = GetPlayerController();
	return Controller ? Cast<ATakeoverVehiclePawn>(Controller->GetPawn()) : nullptr;
}

ATakeoverArena* UMissionSubsystem::FindArena() const
{
	for (TActorIterator<ATakeoverArena> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
