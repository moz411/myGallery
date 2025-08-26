#include "MRUKExtensions.h"
#include "MathUtil.h"
#include "MRUtilityKitAnchor.h"
#include "MRUtilityKitSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Containers/Set.h"

TArray<FVector> GeneratePoints(const FTransform& Plane, const FBox2D& PlaneBounds, double PointsPerUnitX, double PointsPerUnitY, double WorldToMeters = 100.0)
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
			const float Dx = (Ix + 1) * Stride.X;
			const float Dy = (Iy + 1) * Stride.Y;
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

		const auto Points = GeneratePoints(WallAnchor->GetTransform(), WallAnchor->PlaneBounds, PointsPerUnitX, PointsPerUnitY, WorldToMeters);
		AllPoints.Append(Points);
	}

	// Generate points between floor and ceiling
	const float DistFloorCeiling = Room->CeilingAnchor->GetTransform().GetLocation().Z - Room->FloorAnchor->GetTransform().GetLocation().Z;
	const int32 PlanesCount = FMath::Max(FMathf::Ceil(PointsPerUnitY * DistFloorCeiling) / WorldToMeters, 1);
	const int32 SpaceBetweenPlanes = DistFloorCeiling / PlanesCount;
	for (int i = 1; i < PlanesCount; ++i)
	{
		FTransform Transform = Room->CeilingAnchor->GetTransform();
		Transform.SetLocation(FVector(Transform.GetLocation().X, Transform.GetLocation().Y, Transform.GetLocation().Z - (SpaceBetweenPlanes * i)));
		const auto Points = GeneratePoints(Transform, Room->CeilingAnchor->PlaneBounds, PointsPerUnitX, PointsPerUnitY, WorldToMeters);
		AllPoints.Append(Points);
	}

	const auto CeilingPoints = GeneratePoints(Room->CeilingAnchor->GetTransform(), Room->CeilingAnchor->PlaneBounds, PointsPerUnitX, PointsPerUnitY, WorldToMeters);
	AllPoints.Append(CeilingPoints);

	const auto FloorPoints = GeneratePoints(Room->FloorAnchor->GetTransform(), Room->FloorAnchor->PlaneBounds, PointsPerUnitX, PointsPerUnitY, WorldToMeters);
	AllPoints.Append(FloorPoints);

	if (AllPoints.Num() > MaxPointsCount)
	{
		// Shuffle the array
		AllPoints.Sort([](const FVector& /*Item1*/, const FVector& /*Item2*/) {
			return FMath::FRand() < 0.5f;
		});

		// Randomly remove some points
		int32 PointsToRemoveCount = AllPoints.Num() - MaxPointsCount;
		while (PointsToRemoveCount > 0)
		{
			AllPoints.Pop();
			--PointsToRemoveCount;
		}
	}
	return AllPoints;
}
