#include "Vehicle/TakeoverVehiclePawn.h"

#include "Arena/TakeoverScoringComponent.h"
#include "Camera/CameraComponent.h"
#include "ChaosVehicleWheel.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Player/TakeoverInputConfig.h"
#include "Player/TakeoverPlayerController.h"
#include "Takeover.h"
#include "Vehicle/TakeoverHandlingProfile.h"

ATakeoverVehiclePawn::ATakeoverVehiclePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName(TEXT("Vehicle")));
	GetMesh()->SetNotifyRigidBodyCollision(true);

	// Rotation lag lets the camera swing wide in a drift, which shows off the angle
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = 650.f;
	SpringArm->SocketOffset.Z = 150.f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 2.f;
	SpringArm->CameraLagMaxDistance = 50.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	Scoring = CreateDefaultSubobject<UTakeoverScoringComponent>(TEXT("Scoring"));

	// Socket names match the vehicle template's wheel rig
	BodyMesh = CreateVisualMesh(TEXT("BodyMesh"), NAME_None);
	GlassMesh = CreateVisualMesh(TEXT("GlassMesh"), NAME_None);
	WheelMeshFL = CreateVisualMesh(TEXT("WheelMeshFL"), TEXT("Phys_Wheel_FL"));
	WheelMeshFR = CreateVisualMesh(TEXT("WheelMeshFR"), TEXT("Phys_Wheel_FR"));
	WheelMeshRL = CreateVisualMesh(TEXT("WheelMeshRL"), TEXT("Phys_Wheel_BL"));
	WheelMeshRR = CreateVisualMesh(TEXT("WheelMeshRR"), TEXT("Phys_Wheel_BR"));

	// The template's wheel bones point their Y axis up, so the wheel mesh (axle on Y) is turned onto the axle.
	// One wheel mesh serves both sides: left and right turn opposite ways so both rims face outward.
	WheelMeshFL->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	WheelMeshRL->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	WheelMeshFR->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	WheelMeshRR->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
}

UStaticMeshComponent* ATakeoverVehiclePawn::CreateVisualMesh(FName Name, FName ParentSocket)
{
	UStaticMeshComponent* Visual = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Visual->SetupAttachment(GetMesh(), ParentSocket);
	Visual->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Visual->SetGenerateOverlapEvents(false);
	Visual->SetCanEverAffectNavigation(false);
	return Visual;
}

void ATakeoverVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	if (DefaultHandlingProfile)
	{
		ApplyHandlingProfile(DefaultHandlingProfile);
	}
	else
	{
		UE_LOG(LogTakeover, Warning, TEXT("%s has no default handling profile"), *GetName());
	}
}

void ATakeoverVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	const ATakeoverPlayerController* TakeoverController = Cast<ATakeoverPlayerController>(Controller);
	const UTakeoverInputConfig* InputConfig = TakeoverController ? TakeoverController->GetInputConfig() : nullptr;
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!InputConfig || !EnhancedInput)
	{
		UE_LOG(LogTakeover, Warning, TEXT("%s: no input config or Enhanced Input component, vehicle input is unbound"), *GetName());
		return;
	}

	// Completed fires with a zero value, which releases the axis
	EnhancedInput->BindAction(InputConfig->ThrottleAction, ETriggerEvent::Triggered, this, &ATakeoverVehiclePawn::Throttle);
	EnhancedInput->BindAction(InputConfig->ThrottleAction, ETriggerEvent::Completed, this, &ATakeoverVehiclePawn::Throttle);
	EnhancedInput->BindAction(InputConfig->BrakeAction, ETriggerEvent::Triggered, this, &ATakeoverVehiclePawn::Brake);
	EnhancedInput->BindAction(InputConfig->BrakeAction, ETriggerEvent::Completed, this, &ATakeoverVehiclePawn::Brake);
	EnhancedInput->BindAction(InputConfig->SteerAction, ETriggerEvent::Triggered, this, &ATakeoverVehiclePawn::Steer);
	EnhancedInput->BindAction(InputConfig->SteerAction, ETriggerEvent::Completed, this, &ATakeoverVehiclePawn::Steer);
	EnhancedInput->BindAction(InputConfig->HandbrakeAction, ETriggerEvent::Started, this, &ATakeoverVehiclePawn::StartHandbrake);
	EnhancedInput->BindAction(InputConfig->HandbrakeAction, ETriggerEvent::Completed, this, &ATakeoverVehiclePawn::StopHandbrake);
	EnhancedInput->BindAction(InputConfig->LookAroundAction, ETriggerEvent::Triggered, this, &ATakeoverVehiclePawn::LookAround);
	EnhancedInput->BindAction(InputConfig->ResetVehicleAction, ETriggerEvent::Started, this, &ATakeoverVehiclePawn::ResetVehicle);
}

void ATakeoverVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GetMesh()->SetAngularDamping(GetChaosMovement()->IsMovingOnGround() ? 0.f : AirborneAngularDamping);

	const float CameraYaw = static_cast<float>(SpringArm->GetRelativeRotation().Yaw);
	SpringArm->SetRelativeRotation(FRotator(0.f, FMath::FInterpTo(CameraYaw, 0.f, DeltaSeconds, CameraRecenterSpeed), 0.f));
}

void ATakeoverVehiclePawn::ApplyHandlingProfile(UTakeoverHandlingProfile* Profile)
{
	if (!Profile)
	{
		return;
	}

	UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
	Movement->SetMaxEngineTorque(Profile->MaxEngineTorque);
	Movement->SetDragCoefficient(Profile->DragCoefficient);
	Movement->SetDownforceCoefficient(Profile->DownforceCoefficient);

	if (Movement->Wheels.IsEmpty())
	{
		UE_LOG(LogTakeover, Warning, TEXT("%s: wheels not created yet, tire values in %s skipped"), *GetName(), *Profile->GetName());
	}
	for (int32 WheelIndex = 0; WheelIndex < Movement->Wheels.Num(); ++WheelIndex)
	{
		const UChaosVehicleWheel* Wheel = Movement->Wheels[WheelIndex];
		if (!Wheel)
		{
			continue;
		}
		const bool bFront = Wheel->AxleType == EAxleType::Front;
		Movement->SetWheelFrictionMultiplier(WheelIndex, bFront ? Profile->FrontFrictionMultiplier : Profile->RearFrictionMultiplier);
	}

	ActiveHandlingProfile = Profile;
}

UChaosWheeledVehicleMovementComponent* ATakeoverVehiclePawn::GetChaosMovement() const
{
	return CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
}

void ATakeoverVehiclePawn::Throttle(const FInputActionValue& Value)
{
	ThrottleInput = Value.Get<float>();
	GetChaosMovement()->SetThrottleInput(ThrottleInput);
}

void ATakeoverVehiclePawn::Brake(const FInputActionValue& Value)
{
	BrakeInput = Value.Get<float>();
	GetChaosMovement()->SetBrakeInput(BrakeInput);
}

void ATakeoverVehiclePawn::Steer(const FInputActionValue& Value)
{
	GetChaosMovement()->SetSteeringInput(Value.Get<float>());
}

void ATakeoverVehiclePawn::StartHandbrake(const FInputActionValue& Value)
{
	GetChaosMovement()->SetHandbrakeInput(true);
}

void ATakeoverVehiclePawn::StopHandbrake(const FInputActionValue& Value)
{
	GetChaosMovement()->SetHandbrakeInput(false);
}

void ATakeoverVehiclePawn::LookAround(const FInputActionValue& Value)
{
	SpringArm->AddLocalRotation(FRotator(0.f, Value.Get<float>() * LookSensitivity, 0.f));
}

void ATakeoverVehiclePawn::ResetVehicle(const FInputActionValue& Value)
{
	// Upright at the current heading, slightly above the current spot
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.f;
	ResetRotation.Roll = 0.f;
	const FVector ResetLocation = GetActorLocation() + FVector(0.f, 0.f, ResetHeightCm);

	SetActorTransform(FTransform(ResetRotation, ResetLocation), false, nullptr, ETeleportType::TeleportPhysics);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
}
