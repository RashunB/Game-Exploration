#include "UI/TakeoverDebugHUD.h"

#include "Arena/TakeoverScoringComponent.h"
#include "Core/TakeoverProgressSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/Canvas.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Mission/MissionSubsystem.h"
#include "Player/TakeoverCharacter.h"
#include "Player/TakeoverInputConfig.h"
#include "Player/TakeoverPlayerController.h"
#include "Stealth/TakeoverInteractableComponent.h"

namespace
{
	constexpr float MarginPx = 40.f;
	constexpr float LineHeightPx = 22.f;
	const FLinearColor ControlsColor(1.f, 1.f, 1.f, 0.75f);
	const FLinearColor ObjectiveColor(1.f, 0.85f, 0.3f, 1.f);
	constexpr float MissionColumnWidthPx = 460.f;
}

void ATakeoverDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	const APlayerController* PlayerController = GetOwningPlayerController();
	DrawControls(PlayerController);
	DrawMission(PlayerController);

	const APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	const UTakeoverScoringComponent* Scoring = Pawn ? Pawn->FindComponentByClass<UTakeoverScoringComponent>() : nullptr;
	if (!Scoring)
	{
		return;
	}

	const FTakeoverScoringTickResult& Result = Scoring->GetLastResult();
	const FString Lines[] =
	{
		FString::Printf(TEXT("Score  %d"), FMath::RoundToInt(Scoring->GetScore())),
		FString::Printf(TEXT("Combo  x%d"), Scoring->GetCombo()),
		FString::Printf(TEXT("State  %s"), *UEnum::GetDisplayValueAsText(Scoring->GetState()).ToString()),
		FString::Printf(TEXT("Slip   %.0f deg"), Result.SlipAngleDeg),
		FString::Printf(TEXT("Speed  %.0f km/h"), Result.SpeedKmh),
		FString::Printf(TEXT("Crowd  x%.2f"), Result.ProximityMultiplier),
	};

	float Y = MarginPx;
	for (const FString& Line : Lines)
	{
		DrawText(Line, FLinearColor::White, MarginPx, Y);
		Y += LineHeightPx;
	}
}

void ATakeoverDebugHUD::DrawControls(const APlayerController* PlayerController)
{
	const ATakeoverPlayerController* TakeoverController = Cast<ATakeoverPlayerController>(PlayerController);
	const UTakeoverInputConfig* InputConfig = TakeoverController ? TakeoverController->GetInputConfig() : nullptr;
	const UEnhancedInputLocalPlayerSubsystem* InputSubsystem = PlayerController
		? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())
		: nullptr;
	if (!InputConfig || !InputSubsystem || !Canvas)
	{
		return;
	}

	const TPair<const TCHAR*, const UInputAction*> Rows[] =
	{
		{ TEXT("Throttle"), InputConfig->ThrottleAction },
		{ TEXT("Brake / reverse"), InputConfig->BrakeAction },
		{ TEXT("Steer"), InputConfig->SteerAction },
		{ TEXT("Handbrake"), InputConfig->HandbrakeAction },
		{ TEXT("Look around"), InputConfig->LookAroundAction },
		{ TEXT("Reset car"), InputConfig->ResetVehicleAction },
	};

	float Y = Canvas->ClipY - MarginPx - LineHeightPx * UE_ARRAY_COUNT(Rows);
	for (const TPair<const TCHAR*, const UInputAction*>& Row : Rows)
	{
		if (!Row.Value)
		{
			continue;
		}

		TArray<FString> KeyNames;
		for (const FKey& Key : InputSubsystem->QueryKeysMappedToAction(Row.Value))
		{
			KeyNames.Add(Key.GetDisplayName(false).ToString());
		}
		DrawText(FString::Printf(TEXT("%s:  %s"), Row.Key, *FString::Join(KeyNames, TEXT(" / "))), ControlsColor, MarginPx, Y);
		Y += LineHeightPx;
	}
}

void ATakeoverDebugHUD::DrawMission(const APlayerController* PlayerController)
{
	const UWorld* World = GetWorld();
	const UMissionSubsystem* Mission = World ? World->GetSubsystem<UMissionSubsystem>() : nullptr;
	if (!Mission || !Canvas || Mission->GetPhase() == EMissionPhase::Idle)
	{
		return;
	}

	TArray<TPair<FString, FLinearColor>> Lines;
	Lines.Emplace(FString::Printf(TEXT("Phase  %s"), *UEnum::GetDisplayValueAsText(Mission->GetPhase()).ToString()), FLinearColor::White);
	Lines.Emplace(Mission->GetObjectiveText().ToString(), ObjectiveColor);

	if (Mission->IsTakeoverTimerRunning())
	{
		Lines.Emplace(FString::Printf(TEXT("Takeover ends in  %.0f s"), Mission->GetTakeoverSecondsRemaining()), FLinearColor::White);
	}

	if (const ATakeoverCharacter* Character = PlayerController ? Cast<ATakeoverCharacter>(PlayerController->GetPawn()) : nullptr)
	{
		if (const UTakeoverInteractableComponent* Target = Character->FindInteractable())
		{
			Lines.Emplace(FString::Printf(TEXT("[Interact]  %s"), *Target->Prompt.ToString()), FLinearColor::White);
		}
		const FText Hint = Character->GetHint();
		if (!Hint.IsEmpty())
		{
			Lines.Emplace(Hint.ToString(), ObjectiveColor);
		}
	}

	if (const UTakeoverProgressSubsystem* Progress = World->GetGameInstance()->GetSubsystem<UTakeoverProgressSubsystem>())
	{
		Lines.Emplace(FString::Printf(TEXT("Money  $%d   Rep  %d   Loops  %d"), Progress->GetMoney(), Progress->GetRep(), Progress->GetLoopsCompleted()), ControlsColor);
	}

	const float X = Canvas->ClipX - MarginPx - MissionColumnWidthPx;
	float Y = MarginPx;
	for (const TPair<FString, FLinearColor>& Line : Lines)
	{
		DrawText(Line.Key, Line.Value, X, Y);
		Y += LineHeightPx;
	}
}
