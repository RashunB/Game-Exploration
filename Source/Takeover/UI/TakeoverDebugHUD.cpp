#include "UI/TakeoverDebugHUD.h"

#include "Arena/TakeoverScoringComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

namespace
{
	constexpr float MarginPx = 40.f;
	constexpr float LineHeightPx = 22.f;
}

void ATakeoverDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	const APlayerController* PlayerController = GetOwningPlayerController();
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
