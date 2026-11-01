#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Mission/MissionConfig.h"
#include "TakeoverGameMode.generated.h"

/** Default pawn is set on the Blueprint subclass, since it references the character or hero car Blueprint */
UCLASS()
class TAKEOVER_API ATakeoverGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATakeoverGameMode();

	const UMissionConfig* GetMissionConfig() const { return MissionConfig; }

protected:
	/** Without it, UMissionSubsystem runs the loop on UMissionConfig class defaults */
	UPROPERTY(EditDefaultsOnly, Category = "Mission")
	TObjectPtr<UMissionConfig> MissionConfig;
};
