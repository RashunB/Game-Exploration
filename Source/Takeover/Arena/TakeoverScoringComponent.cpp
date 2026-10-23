#include "Arena/TakeoverScoringComponent.h"

#include "Arena/TakeoverArena.h"
#include "Arena/TakeoverScoringConfig.h"
#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"
#include "Takeover.h"
#include "Vehicle/TakeoverVehiclePawn.h"

UTakeoverScoringComponent::UTakeoverScoringComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTakeoverScoringComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!Config)
	{
		UE_LOG(LogTakeover, Warning, TEXT("%s on %s has no scoring config, using class defaults"), *GetName(), *GetNameSafe(GetOwner()));
	}

	if (!Arena)
	{
		for (TActorIterator<ATakeoverArena> It(GetWorld()); It; ++It)
		{
			Arena = *It;
			break;
		}
	}

	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent()))
	{
		Root->SetNotifyRigidBodyCollision(true);
		Root->OnComponentHit.AddUniqueDynamic(this, &UTakeoverScoringComponent::HandleOwnerHit);
	}
}

void UTakeoverScoringComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const UTakeoverScoringConfig* ActiveConfig = GetActiveConfig();

	FTakeoverVehicleSample Sample = BuildSample();
	Sample.bCollided = bPendingCollision;

	CrowdHitCooldownRemaining = FMath::Max(CrowdHitCooldownRemaining - DeltaTime, 0.f);
	if (bPendingCrowdHit && CrowdHitCooldownRemaining <= 0.f)
	{
		Sample.bHitCrowd = true;
		CrowdHitCooldownRemaining = ActiveConfig->CrowdHitCooldownSeconds;
	}

	bPendingCollision = false;
	bPendingCrowdHit = false;

	const ETakeoverScoringState PreviousState = Model.GetState();
	const int32 PreviousCombo = Model.GetCombo();

	LastResult = Model.Tick(ActiveConfig->Params, Sample, DeltaTime);

	if (LastResult.bHitCrowd)
	{
		OnCrowdHit.Broadcast();
	}
	if (Model.GetCombo() != PreviousCombo)
	{
		OnComboChanged.Broadcast(Model.GetCombo());
	}
	if (Model.GetState() != PreviousState)
	{
		OnStateChanged.Broadcast(Model.GetState());
	}
}

void UTakeoverScoringComponent::SetArena(ATakeoverArena* InArena)
{
	Arena = InArena;
}

void UTakeoverScoringComponent::ResetScore()
{
	Model.Reset();
	LastResult = FTakeoverScoringTickResult();
}

void UTakeoverScoringComponent::HandleOwnerHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Any contact with the crowd counts, however soft
	if (const ATakeoverArena* HitArena = Cast<ATakeoverArena>(OtherActor))
	{
		if (HitArena->IsCrowdComponent(OtherComp))
		{
			bPendingCrowdHit = true;
			return;
		}
	}

	const UTakeoverScoringConfig* ActiveConfig = GetActiveConfig();
	if (Hit.ImpactNormal.Z >= ActiveConfig->GroundNormalMinZ || NormalImpulse.Size() < ActiveConfig->MinCollisionImpulse)
	{
		return;
	}

	bPendingCollision = true;
}

const UTakeoverScoringConfig* UTakeoverScoringComponent::GetActiveConfig() const
{
	return Config ? Config.Get() : GetDefault<UTakeoverScoringConfig>();
}

FTakeoverVehicleSample UTakeoverScoringComponent::BuildSample() const
{
	FTakeoverVehicleSample Sample;

	const AActor* Owner = GetOwner();
	const FVector Forward = Owner->GetActorForwardVector();
	const FVector Velocity = Owner->GetVelocity();
	Sample.Forward = FVector2D(Forward.X, Forward.Y).GetSafeNormal();
	Sample.Velocity = FVector2D(Velocity.X, Velocity.Y);

	if (const UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
	{
		Sample.YawRateDegPerSec = static_cast<float>(Root->GetPhysicsAngularVelocityInDegrees().Z);
	}

	if (const ATakeoverVehiclePawn* Vehicle = Cast<ATakeoverVehiclePawn>(Owner))
	{
		Sample.Throttle = Vehicle->GetThrottleInput();
		Sample.Brake = Vehicle->GetBrakeInput();
	}

	if (Arena)
	{
		Sample.DistanceToCrowdCm = Arena->GetDistanceToCrowd(Owner->GetActorLocation());
	}

	return Sample;
}
