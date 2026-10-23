#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TakeoverInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

/** Points C++ at the Input Action and Mapping Context assets created in the editor */
UCLASS(BlueprintType)
class TAKEOVER_API UTakeoverInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Contexts")
	TObjectPtr<UInputMappingContext> VehicleContext;

	/** Axis1D, 0 to 1 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<UInputAction> ThrottleAction;

	/** Axis1D, 0 to 1. Reverses once the car is stopped. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<UInputAction> BrakeAction;

	/** Axis1D, -1 to 1 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<UInputAction> SteerAction;

	/** Digital */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<UInputAction> HandbrakeAction;

	/** Axis1D, camera yaw */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<UInputAction> LookAroundAction;

	/** Digital */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<UInputAction> ResetVehicleAction;
};
