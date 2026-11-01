#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TakeoverCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UTakeoverInteractableComponent;
struct FInputActionValue;

/**
 * The on-foot player for the break-in: third-person movement, a chase camera, and interaction with
 * UTakeoverInteractableComponent (key spots, the car door). Mesh and animation are assigned on the Blueprint subclass.
 * POC: no noise, sprint, or crouch yet (PLAN.md 5.1).
 */
UCLASS()
class TAKEOVER_API ATakeoverCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATakeoverCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void GiveKeys() { bHasKeys = true; }
	bool HasKeys() const { return bHasKeys; }

	/** The nearest enabled interactable within reach, or null */
	UTakeoverInteractableComponent* FindInteractable() const;

	/** A short message on the HUD, such as "Nothing here." */
	void ShowHint(const FText& Hint);
	FText GetHint() const;

	/** Called by the car when the player gets in: the body stays behind, hidden and out of the way */
	void StowForDriving();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	/** Distance from the character to an interactable's location */
	UPROPERTY(EditDefaultsOnly, Category = "Interact", meta = (ClampMin = "0"))
	float InteractReachCm = 250.f;

	UPROPERTY(EditDefaultsOnly, Category = "Interact", meta = (ClampMin = "0"))
	float HintSeconds = 3.f;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);

	bool bHasKeys = false;
	FText CurrentHint;
	double HintExpiresAt = 0.0;
};
