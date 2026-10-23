#include "Core/TakeoverGameMode.h"

#include "Player/TakeoverPlayerController.h"
#include "UI/TakeoverDebugHUD.h"

ATakeoverGameMode::ATakeoverGameMode()
{
	PlayerControllerClass = ATakeoverPlayerController::StaticClass();
	HUDClass = ATakeoverDebugHUD::StaticClass();
}
