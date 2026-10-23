#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TakeoverDebugHUD.generated.h"

/** Plain-text scoring readout for greybox playtests. Replaced by UMG widgets in M2. */
UCLASS()
class TAKEOVER_API ATakeoverDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
