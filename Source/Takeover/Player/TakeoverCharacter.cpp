#include "Player/TakeoverCharacter.h"

#include "Camera/CameraComponent.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Player/TakeoverInputConfig.h"
#include "Player/TakeoverPlayerController.h"
#include "Stealth/TakeoverInteractableComponent.h"
#include "Takeover.h"


ATakeoverCharacter::ATakeoverCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// The camera follows the controller; the body turns to face where it walks
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 400.f;
	SpringArm->SocketOffset.Z = 60.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;
}

void ATakeoverCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	const ATakeoverPlayerController* TakeoverController = Cast<ATakeoverPlayerController>(Controller);
	const UTakeoverInputConfig* InputConfig = TakeoverController ? TakeoverController->GetInputConfig() : nullptr;
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!InputConfig || !EnhancedInput || !InputConfig->MoveAction)
	{
		UE_LOG(LogTakeover, Warning, TEXT("%s: no on-foot input actions in the input config, on-foot input is unbound"), *GetName());
		return;
	}

	EnhancedInput->BindAction(InputConfig->MoveAction, ETriggerEvent::Triggered, this, &ATakeoverCharacter::Move);
	EnhancedInput->BindAction(InputConfig->LookAction, ETriggerEvent::Triggered, this, &ATakeoverCharacter::Look);
	EnhancedInput->BindAction(InputConfig->InteractAction, ETriggerEvent::Started, this, &ATakeoverCharacter::Interact);
}

void ATakeoverCharacter::Move(const FInputActionValue& Value)
{
	// X strafes, Y walks forward, both relative to where the camera faces
	const FVector2D Input = Value.Get<FVector2D>();
	const FRotator Yaw(0.f, GetControlRotation().Yaw, 0.f);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), Input.Y);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), Input.X);
}

void ATakeoverCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

void ATakeoverCharacter::Interact(const FInputActionValue& Value)
{
	if (UTakeoverInteractableComponent* Target = FindInteractable())
	{
		Target->Interact(this);
	}
}

UTakeoverInteractableComponent* ATakeoverCharacter::FindInteractable() const
{
	UTakeoverInteractableComponent* Best = nullptr;
	double BestDistanceSq = FMath::Square(static_cast<double>(InteractReachCm));
	const FVector Location = GetActorLocation();

	// A handful of interactables per level, so a full pass is cheap enough for the POC
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (*It == this)
		{
			continue;
		}

		TInlineComponentArray<UTakeoverInteractableComponent*> Interactables(*It);
		for (UTakeoverInteractableComponent* Candidate : Interactables)
		{
			const double DistanceSq = FVector::DistSquared(Candidate->GetComponentLocation(), Location);
			if (Candidate->bEnabled && DistanceSq <= BestDistanceSq)
			{
				Best = Candidate;
				BestDistanceSq = DistanceSq;
			}
		}
	}
	return Best;
}

void ATakeoverCharacter::ShowHint(const FText& Hint)
{
	CurrentHint = Hint;
	HintExpiresAt = GetWorld()->GetTimeSeconds() + HintSeconds;
}

FText ATakeoverCharacter::GetHint() const
{
	return GetWorld()->GetTimeSeconds() < HintExpiresAt ? CurrentHint : FText::GetEmpty();
}

void ATakeoverCharacter::StowForDriving()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	GetCharacterMovement()->DisableMovement();
}
