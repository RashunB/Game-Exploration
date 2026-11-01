#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TakeoverKeySpot.generated.h"

class ATakeoverCharacter;
class UStaticMeshComponent;
class UTakeoverInteractableComponent;

/**
 * A place in the house where the car keys might be (a drawer, a key hook). UMissionSubsystem hides the keys
 * at one spot per loop; searching any other spot finds nothing. Meshes are assigned on the Blueprint subclass.
 */
UCLASS()
class TAKEOVER_API ATakeoverKeySpot : public AActor
{
	GENERATED_BODY()

public:
	ATakeoverKeySpot();

	/** Makes the spot searchable again, with or without the keys */
	void ResetSearch(bool bInHasKeys);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Key spot")
	TObjectPtr<USceneComponent> Root;

	/** The furniture: drawer, cabinet, hook board */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Key spot")
	TObjectPtr<UStaticMeshComponent> SpotMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Key spot")
	TObjectPtr<UTakeoverInteractableComponent> Interactable;

private:
	UFUNCTION()
	void HandleSearch(ATakeoverCharacter* Character);

	bool bHasKeys = false;
};
