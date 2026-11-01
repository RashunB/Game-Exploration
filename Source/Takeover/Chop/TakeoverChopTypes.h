#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TakeoverChopTypes.generated.h"

/** One removable part, as a DataTable row. The row name is the part id. */
USTRUCT(BlueprintType)
struct TAKEOVER_API FTakeoverChopPartRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chop")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chop", meta = (ClampMin = "0"))
	int32 BaseValue = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chop", meta = (ClampMin = "0"))
	float RemovalSeconds = 2.f;
};
