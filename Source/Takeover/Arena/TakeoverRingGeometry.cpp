#include "Arena/TakeoverRingGeometry.h"

bool FTakeoverRingGeometry::IsInside(const TArray<FVector2D>& Polygon, const FVector2D& Point)
{
	if (Polygon.Num() < 3)
	{
		return false;
	}

	// Cast a ray toward +X and count edge crossings; an odd count means inside
	bool bInside = false;
	for (int32 Current = 0, Previous = Polygon.Num() - 1; Current < Polygon.Num(); Previous = Current++)
	{
		const FVector2D& A = Polygon[Current];
		const FVector2D& B = Polygon[Previous];
		if ((A.Y > Point.Y) != (B.Y > Point.Y))
		{
			const double CrossingX = A.X + (Point.Y - A.Y) * (B.X - A.X) / (B.Y - A.Y);
			if (Point.X < CrossingX)
			{
				bInside = !bInside;
			}
		}
	}
	return bInside;
}
