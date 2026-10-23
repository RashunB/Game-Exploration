#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TakeoverPlayerController.generated.h"

class UTakeoverInputConfig;

/**
 * Swaps input mapping contexts to match the possessed pawn.
 * Heat and on-foot/vehicle possession swaps land here in M2 and M3.
 */
UCLASS()
class TAKEOVER_API ATakeoverPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	const UTakeoverInputConfig* GetInputConfig() const { return InputConfig; }

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UTakeoverInputConfig> InputConfig;

private:
	void ApplyMappingContextFor(const APawn* InPawn);
};
