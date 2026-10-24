#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "TakeoverVehiclePawn.generated.h"

class UCameraComponent;
class UChaosWheeledVehicleMovementComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UTakeoverHandlingProfile;
class UTakeoverScoringComponent;
struct FInputActionValue;

/**
 * Player-drivable Chaos vehicle. Mesh, wheels, and tuning assets are assigned on the Blueprint subclass.
 */
UCLASS(Abstract)
class TAKEOVER_API ATakeoverVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	ATakeoverVehiclePawn();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Takeover|Vehicle")
	void ApplyHandlingProfile(UTakeoverHandlingProfile* Profile);

	UTakeoverHandlingProfile* GetActiveHandlingProfile() const { return ActiveHandlingProfile; }

	/** Driver input as last received, 0 to 1 */
	float GetThrottleInput() const { return ThrottleInput; }
	float GetBrakeInput() const { return BrakeInput; }

	UTakeoverScoringComponent* GetScoring() const { return Scoring; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoring")
	TObjectPtr<UTakeoverScoringComponent> Scoring;

	/**
	 * Visual parts for cars whose skeletal mesh only carries the wheel rig, like the vehicle template's
	 * SportsCar. Visual only, no collision. Leave a mesh unset on the Blueprint to hide that part.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> GlassMesh;

	/** Attached to the wheel bones so they spin and steer with the rig. Change the parent socket on the Blueprint for other rigs. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> WheelMeshFL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> WheelMeshFR;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> WheelMeshRL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> WheelMeshRR;

	/** Applied on BeginPlay */
	UPROPERTY(EditDefaultsOnly, Category = "Handling")
	TObjectPtr<UTakeoverHandlingProfile> DefaultHandlingProfile;

	/** Degrees of camera yaw per unit of look input */
	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (ClampMin = "0"))
	float LookSensitivity = 1.f;

	/** How fast the camera swings back behind the car after looking around */
	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (ClampMin = "0"))
	float CameraRecenterSpeed = 1.f;

	/** Damps tumbling in the air so jumps land wheels-down */
	UPROPERTY(EditDefaultsOnly, Category = "Physics", meta = (ClampMin = "0"))
	float AirborneAngularDamping = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle", meta = (ClampMin = "0"))
	float ResetHeightCm = 50.f;

private:
	UChaosWheeledVehicleMovementComponent* GetChaosMovement() const;
	UStaticMeshComponent* CreateVisualMesh(FName Name, FName ParentSocket);

	void Throttle(const FInputActionValue& Value);
	void Brake(const FInputActionValue& Value);
	void Steer(const FInputActionValue& Value);
	void StartHandbrake(const FInputActionValue& Value);
	void StopHandbrake(const FInputActionValue& Value);
	void LookAround(const FInputActionValue& Value);
	void ResetVehicle(const FInputActionValue& Value);

	UPROPERTY(Transient)
	TObjectPtr<UTakeoverHandlingProfile> ActiveHandlingProfile;

	float ThrottleInput = 0.f;
	float BrakeInput = 0.f;
};
