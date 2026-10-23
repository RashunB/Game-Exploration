#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TakeoverGameMode.generated.h"

/** Default pawn is set on the Blueprint subclass, since it references the hero car Blueprint */
UCLASS()
class TAKEOVER_API ATakeoverGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATakeoverGameMode();
};
