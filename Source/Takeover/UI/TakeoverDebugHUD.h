#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TakeoverDebugHUD.generated.h"

/** Plain-text scoring, mission, and controls readout for greybox playtests. Replaced by UMG widgets later. */
UCLASS()
class TAKEOVER_API ATakeoverDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	/** Lists the keys currently mapped to each vehicle action, so the legend follows the mapping context */
	void DrawControls(const APlayerController* PlayerController);

	/** Mission phase, objective, takeover time left, interact prompt, hint, and money and rep, top right */
	void DrawMission(const APlayerController* PlayerController);
};
