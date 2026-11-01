#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TakeoverInteractableComponent.generated.h"

class ATakeoverCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeoverInteractSignature, ATakeoverCharacter*, Character);

/**
 * Something the on-foot player can use: a drawer to search, a car door. The owner reacts in OnInteract.
 * ATakeoverCharacter uses the nearest enabled one within reach. PLAN.md 5.1 asks for hold-to-interact; the POC uses a press.
 */
UCLASS(ClassGroup = (Takeover), meta = (BlueprintSpawnableComponent))
class TAKEOVER_API UTakeoverInteractableComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	/** Shown on the HUD while this is the one the player would use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	FText Prompt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	bool bEnabled = true;

	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FTakeoverInteractSignature OnInteract;

	void Interact(ATakeoverCharacter* Character);
};
