#include "UI/TakeoverDebugHUD.h"

#include "Arena/TakeoverScoringComponent.h"
#include "Engine/Canvas.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Player/TakeoverInputConfig.h"
#include "Player/TakeoverPlayerController.h"

namespace
{
	constexpr float MarginPx = 40.f;
	constexpr float LineHeightPx = 22.f;
	const FLinearColor ControlsColor(1.f, 1.f, 1.f, 0.75f);
}

void ATakeoverDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	const APlayerController* PlayerController = GetOwningPlayerController();
	DrawControls(PlayerController);

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
