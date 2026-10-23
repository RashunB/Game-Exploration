#include "Arena/TakeoverArena.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/CollisionProfile.h"

namespace
{
	constexpr int32 DefaultRingPoints = 8;
	constexpr float DefaultRingRadiusCm = 2500.f;
}

ATakeoverArena::ATakeoverArena()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Default to a closed ring so a freshly placed arena is usable; reshape it in the level
	CrowdSpline = CreateDefaultSubobject<USplineComponent>(TEXT("CrowdSpline"));
	CrowdSpline->SetupAttachment(Root);
	CrowdSpline->ClearSplinePoints(false);
	for (int32 PointIndex = 0; PointIndex < DefaultRingPoints; ++PointIndex)
	{
		const float Angle = UE_TWO_PI * PointIndex / DefaultRingPoints;
		CrowdSpline->AddSplinePoint(FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * DefaultRingRadiusCm, ESplineCoordinateSpace::Local, false);
	}
	CrowdSpline->SetClosedLoop(true, false);
	CrowdSpline->UpdateSpline();

	CrowdPlaceholders = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CrowdPlaceholders"));
	CrowdPlaceholders->SetupAttachment(Root);
	CrowdPlaceholders->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
}

void ATakeoverArena::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CrowdPlaceholders->ClearInstances();

	const float SplineLength = CrowdSpline->GetSplineLength();
	const int32 Count = FMath::FloorToInt(SplineLength / PlaceholderSpacingCm);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const float Distance = Index * PlaceholderSpacingCm;
		const FVector Location = CrowdSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		const FRotator Rotation = CrowdSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		CrowdPlaceholders->AddInstance(FTransform(Rotation, Location), true);
	}
}

float ATakeoverArena::GetDistanceToCrowd(const FVector& WorldLocation) const
{
	const FVector Closest = CrowdSpline->FindLocationClosestToWorldLocation(WorldLocation, ESplineCoordinateSpace::World);
	return static_cast<float>(FVector::Dist2D(Closest, WorldLocation));
}

bool ATakeoverArena::IsCrowdComponent(const UPrimitiveComponent* Component) const
{
	return Component && Component == CrowdPlaceholders.Get();
}
