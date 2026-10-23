#pragma once

#include "CoreMinimal.h"
#include "Arena/TakeoverScoringModel.h"
#include "Components/ActorComponent.h"
#include "TakeoverScoringComponent.generated.h"

class ATakeoverArena;
class UTakeoverScoringConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeoverComboChangedSignature, int32, Combo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeoverScoringStateChangedSignature, ETakeoverScoringState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTakeoverCrowdHitSignature);

/** Samples the owning vehicle each tick and feeds FTakeoverScoringModel */
UCLASS(ClassGroup = (Takeover), meta = (BlueprintSpawnableComponent))
class TAKEOVER_API UTakeoverScoringComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTakeoverScoringComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Arena whose crowd line drives the proximity multiplier. Falls back to the first arena in the world. */
	void SetArena(ATakeoverArena* InArena);

	UFUNCTION(BlueprintCallable, Category = "Takeover|Scoring")
	void ResetScore();

	UFUNCTION(BlueprintPure, Category = "Takeover|Scoring")
	float GetScore() const { return Model.GetScore(); }

	UFUNCTION(BlueprintPure, Category = "Takeover|Scoring")
	int32 GetCombo() const { return Model.GetCombo(); }

	UFUNCTION(BlueprintPure, Category = "Takeover|Scoring")
	ETakeoverScoringState GetState() const { return Model.GetState(); }

	const FTakeoverScoringTickResult& GetLastResult() const { return LastResult; }

	UPROPERTY(BlueprintAssignable, Category = "Takeover|Scoring")
	FTakeoverComboChangedSignature OnComboChanged;

	UPROPERTY(BlueprintAssignable, Category = "Takeover|Scoring")
	FTakeoverScoringStateChangedSignature OnStateChanged;

	/** Heat hooks in here in M2 */
	UPROPERTY(BlueprintAssignable, Category = "Takeover|Scoring")
	FTakeoverCrowdHitSignature OnCrowdHit;

protected:
	virtual void BeginPlay() override;

	/** Falls back to the class defaults of UTakeoverScoringConfig when unset */
	UPROPERTY(EditDefaultsOnly, Category = "Scoring")
	TObjectPtr<UTakeoverScoringConfig> Config;

private:
	UFUNCTION()
	void HandleOwnerHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	const UTakeoverScoringConfig* GetActiveConfig() const;
	FTakeoverVehicleSample BuildSample() const;

	UPROPERTY(Transient)
	TObjectPtr<ATakeoverArena> Arena;

	FTakeoverScoringModel Model;
	FTakeoverScoringTickResult LastResult;

	bool bPendingCollision = false;
	bool bPendingCrowdHit = false;
	float CrowdHitCooldownRemaining = 0.f;
};
