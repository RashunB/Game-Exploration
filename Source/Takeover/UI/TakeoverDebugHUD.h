#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TakeoverDebugHUD.generated.h"

/** Plain-text scoring readout and controls legend for greybox playtests. Replaced by UMG widgets in M2. */
UCLASS()
class TAKEOVER_API ATakeoverDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	/** Lists the keys currently mapped to each vehicle action, so the legend follows the mapping context */
	void DrawControls(const APlayerController* PlayerController);
};
