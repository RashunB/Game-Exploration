#pragma once

#include "CoreMinimal.h"

/** Ground-plane geometry for the crowd ring, with no world access, so tests drive it directly */
struct TAKEOVER_API FTakeoverRingGeometry
{
	/** Even-odd test against a closed polygon (the last point joins the first). Fewer than 3 points is never inside. */
	static bool IsInside(const TArray<FVector2D>& Polygon, const FVector2D& Point);
};
