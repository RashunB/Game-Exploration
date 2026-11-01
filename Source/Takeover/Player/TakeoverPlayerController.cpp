#include "Player/TakeoverPlayerController.h"

#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "Player/TakeoverCharacter.h"
#include "Player/TakeoverInputConfig.h"
#include "Takeover.h"
#include "UI/TakeoverChopMenuWidget.h"
#include "Vehicle/TakeoverVehiclePawn.h"

ATakeoverPlayerController::ATakeoverPlayerController()
{
	ChopMenuClass = UTakeoverChopMenuWidget::StaticClass();
}

void ATakeoverPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ApplyMappingContextFor(GetPawn());
}

void ATakeoverPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ApplyMappingContextFor(InPawn);
}

void ATakeoverPlayerController::ApplyMappingContextFor(const APawn* InPawn)
{
	if (!InputConfig)
	{
		UE_LOG(LogTakeover, Warning, TEXT("%s has no input config"), *GetName());
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!InputSubsystem)
	{
		return;
	}

	InputSubsystem->ClearAllMappings();
	if (Cast<ATakeoverVehiclePawn>(InPawn) && InputConfig->VehicleContext)
	{
		InputSubsystem->AddMappingContext(InputConfig->VehicleContext, 0);
	}
	else if (Cast<ATakeoverCharacter>(InPawn) && InputConfig->OnFootContext)
	{
		InputSubsystem->AddMappingContext(InputConfig->OnFootContext, 0);
	}
}

void ATakeoverPlayerController::OpenChopMenu()
{
	if (ChopMenu || !ChopMenuClass)
	{
		return;
	}

	ChopMenu = CreateWidget<UTakeoverChopMenuWidget>(this, ChopMenuClass);
	ChopMenu->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ChopMenu->TakeWidget());
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void ATakeoverPlayerController::CloseChopMenu()
{
	if (!ChopMenu)
	{
		return;
	}

	ChopMenu->RemoveFromParent();
	ChopMenu = nullptr;
	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
}
