#include "Core/TakeoverProgressSubsystem.h"

#include "Takeover.h"

void UTakeoverProgressSubsystem::AddMoney(int32 Amount)
{
	Money += FMath::Max(Amount, 0);
	UE_LOG(LogTakeover, Log, TEXT("Money +%d = %d"), Amount, Money);
}

void UTakeoverProgressSubsystem::AddRep(int32 Amount)
{
	Rep += FMath::Max(Amount, 0);
	UE_LOG(LogTakeover, Log, TEXT("Rep +%d = %d"), Amount, Rep);
}
