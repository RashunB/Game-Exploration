#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TakeoverArena.generated.h"

class UInstancedStaticMeshComponent;
class UPrimitiveComponent;
class USplineComponent;

/**
 * Takeover arena: a crowd line defined by a spline, with placeholder crowd meshes along it.
 * The placeholder mesh is assigned on the Blueprint subclass.
 */
UCLASS()
class TAKEOVER_API ATakeoverArena : public AActor
{
	GENERATED_BODY()

public:
	ATakeoverArena();

	virtual void OnConstruction(const FTransform& Transform) override;

	/** Ground-plane distance from a world location to the crowd line */
	float GetDistanceToCrowd(const FVector& WorldLocation) const;

	bool IsCrowdComponent(const UPrimitiveComponent* Component) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	TObjectPtr<USplineComponent> CrowdSpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	TObjectPtr<UInstancedStaticMeshComponent> CrowdPlaceholders;

	/** Per placed arena, since it depends on the spline drawn in the level */
	UPROPERTY(EditAnywhere, Category = "Arena", meta = (ClampMin = "50"))
	float PlaceholderSpacingCm = 150.f;
};
