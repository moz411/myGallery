#include "MRUKExtensions.h"
#include "MathUtil.h"
#include "MRUtilityKitAnchor.h"
#include "MRUtilityKitSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Containers/Set.h"

TArray<FVector> GeneratePoints(const FTransform& Plane, const FBox2D& PlaneBounds,
	double PointsPerUnitX, double PointsPerUnitY, double WorldToMeters = 100.0)
{
	const FVector PlaneRight = Plane.GetRotation().GetRightVector();
	const FVector PlaneUp = Plane.GetRotation().GetUpVector();
	const FVector PlaneSize = FVector(PlaneBounds.GetSize().X, PlaneBounds.GetSize().Y, 0.0);
	const FVector PlaneBottomLeft = Plane.GetLocation() - PlaneRight * PlaneSize.X * 0.5f - PlaneUp * PlaneSize.Y * 0.5f;

	const int32 PointsX = FMath::Max(FMathf::Ceil(PointsPerUnitX * PlaneSize.X) / WorldToMeters, 1);
	const int32 PointsY = FMath::Max(FMathf::Ceil(PointsPerUnitY * PlaneSize.Y) / WorldToMeters, 1);

	const FVector2D Stride{ PlaneSize.X / (PointsX + 1), PlaneSize.Y / (PointsY + 1) };

	TArray<FVector> Points;
	Points.SetNum(PointsX * PointsY);

	for (int Iy = 0; Iy < PointsY; ++Iy)
	{
		for (int Ix = 0; Ix < PointsX; ++Ix)
		{
			const float Dx = Ix * Stride.X;
			const float Dy = Iy * Stride.Y;
			const FVector Point = PlaneBottomLeft + Dx * PlaneRight + Dy * PlaneUp;
			Points[Ix + Iy * PointsX] = Point;
		}
	}

	return Points;
}

TArray<FVector> UMRUKExtensions::ComputeRoomBoxGridSurfaceOnly(
	const AMRUKRoom* Room, int32 MaxPointsCount, double PointsPerUnitX, double PointsPerUnitY)
{
	TArray<FVector> AllPoints;

	const double WorldToMeters = Room->GetWorld()->GetWorldSettings()->WorldToMeters;
	for (const AMRUKAnchor* WallAnchor : Room->WallAnchors)
	{

		const auto Points = GeneratePoints(WallAnchor->GetTransform(), WallAnchor->PlaneBounds,
			PointsPerUnitX, PointsPerUnitY, WorldToMeters);
		AllPoints.Append(Points);
	}

	return AllPoints;
}
