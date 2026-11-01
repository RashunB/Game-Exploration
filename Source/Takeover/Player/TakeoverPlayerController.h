#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TakeoverPlayerController.generated.h"

class UTakeoverChopMenuWidget;
class UTakeoverInputConfig;

/**
 * Swaps input mapping contexts to match the possessed pawn (on foot or driving), and shows the chop menu.
 * Heat lands here later (ADR 0005).
 */
UCLASS()
class TAKEOVER_API ATakeoverPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATakeoverPlayerController();

	const UTakeoverInputConfig* GetInputConfig() const { return InputConfig; }

	/** Shows the chop menu and gives it mouse and gamepad focus */
	void OpenChopMenu();
	void CloseChopMenu();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UTakeoverInputConfig> InputConfig;

	/** Builds its own layout in C++; a Widget Blueprint subclass can replace it for a designed menu */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTakeoverChopMenuWidget> ChopMenuClass;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTakeoverChopMenuWidget> ChopMenu;

	void ApplyMappingContextFor(const APawn* InPawn);
};
