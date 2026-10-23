#include "Player/TakeoverPlayerController.h"

#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "Player/TakeoverInputConfig.h"
#include "Takeover.h"
#include "Vehicle/TakeoverVehiclePawn.h"

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
}
