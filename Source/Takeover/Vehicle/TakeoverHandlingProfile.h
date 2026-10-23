#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TakeoverHandlingProfile.generated.h"

/**
 * Handling values applied to a running vehicle through Chaos runtime setters.
 * Settings Chaos only reads when it creates the vehicle (suspension, wheel classes, steering and
 * handbrake per wheel) stay on the vehicle Blueprint until profile swaps need them.
 */
UCLASS(BlueprintType)
class TAKEOVER_API UTakeoverHandlingProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Nm, multiplied by the engine torque curve */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Engine", meta = (ClampMin = "0"))
	float MaxEngineTorque = 750.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aerodynamics", meta = (ClampMin = "0"))
	float DragCoefficient = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aerodynamics", meta = (ClampMin = "0"))
	float DownforceCoefficient = 0.3f;

	/** Grip on front-axle wheels. Rear grip below front is what lets the car hold a drift. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tires", meta = (ClampMin = "0"))
	float FrontFrictionMultiplier = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tires", meta = (ClampMin = "0"))
	float RearFrictionMultiplier = 2.f;
};
