#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TakeoverProgressSubsystem.generated.h"

/**
 * Money, rep, and loops completed, kept across loops for the life of the game instance.
 * Saving to UTakeoverSaveGame lands in M2; in the POC progress resets when the game closes.
 */
UCLASS()
class TAKEOVER_API UTakeoverProgressSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Takeover|Progress")
	void AddMoney(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Takeover|Progress")
	void AddRep(int32 Amount);

	void RecordLoopCompleted() { ++LoopsCompleted; }

	UFUNCTION(BlueprintPure, Category = "Takeover|Progress")
	int32 GetMoney() const { return Money; }

	UFUNCTION(BlueprintPure, Category = "Takeover|Progress")
	int32 GetRep() const { return Rep; }

	UFUNCTION(BlueprintPure, Category = "Takeover|Progress")
	int32 GetLoopsCompleted() const { return LoopsCompleted; }

private:
	int32 Money = 0;
	int32 Rep = 0;
	int32 LoopsCompleted = 0;
};
