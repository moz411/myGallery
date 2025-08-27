#include "MRUKExtensions.h"
#include "MRUtilityKitAnchor.h"
#include "MRUtilityKitSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Containers/Set.h"

TArray<FTransform> GeneratePoints(const FTransform& Plane, const FBox2D& PlaneBounds,
	double PointsPerUnitX, double PointsPerUnitY, double WorldToMeters, bool bFlipNormal=false)
{
	const FQuat   Q = Plane.GetRotation();
	const FVector R = Q.GetRightVector();
	const FVector U = Q.GetUpVector();
	FVector N = FVector::CrossProduct(R, U).GetSafeNormal();
	if (bFlipNormal) N *= -1.f;

	const FVector2D PlaneSize2D = PlaneBounds.GetSize();
	const FVector  BL = Plane.GetLocation() - R * PlaneSize2D.X * 0.5f - U * PlaneSize2D.Y * 0.5f;

	const int32 PointsX = FMath::Max(FMath::CeilToInt(PointsPerUnitX * (PlaneSize2D.X / WorldToMeters)) - 1, 1);
	const int32 PointsY = FMath::Max(FMath::CeilToInt(PointsPerUnitY * (PlaneSize2D.Y / WorldToMeters)) - 1, 1);

	const FVector2D Stride{ PlaneSize2D.X / (PointsX + 1), PlaneSize2D.Y / (PointsY + 1) };

	// Rotation telle que la normale du plane devienne l’axe Z de l’objet
	const FQuat Qplane = FRotationMatrix::MakeFromXZ(N, U).ToQuat();

	TArray<FTransform> Xforms;
	Xforms.Reserve(PointsX * PointsY);

	for (int32 iy = 0; iy < PointsY; ++iy)
	{
		for (int32 ix = 0; ix < PointsX; ++ix)
		{
			const float dx = (ix + 1) * Stride.X;
			const float dy = (iy + 1) * Stride.Y;
			const FVector P = BL + dx * R + dy * U;

			Xforms.Emplace(Qplane, P, FVector(1.0f));
		}
	}
	return Xforms;
}


// --- ComputeRoomBoxGridSurfaceOnly -> TArray<FTransform> ----------------------
TArray<FTransform> UMRUKExtensions::ComputeRoomBoxGridSurfaceOnly(
    const AMRUKRoom* Room, int32 MaxPointsCount, double PointsPerUnitX, double PointsPerUnitY)
{
	TArray<FTransform> All;
	if (!Room || !Room->GetWorld() || !Room->GetWorld()->GetWorldSettings()) return All;

	const double W2M = Room->GetWorld()->GetWorldSettings()->WorldToMeters;

	TArray<const AMRUKAnchor*> Anchors;
	Anchors.Append(Room->WallAnchors);
	if (Room->CeilingAnchor) Anchors.Add(Room->CeilingAnchor);
	if (Room->FloorAnchor)   Anchors.Add(Room->FloorAnchor);

	for (const AMRUKAnchor* A : Anchors)
	{
		if (!A) continue;
		All.Append(GeneratePoints(A->GetTransform(), A->PlaneBounds,
		                          PointsPerUnitX, PointsPerUnitY, W2M, true));
	}

	if (MaxPointsCount > 0 && All.Num() > MaxPointsCount)
	{
		All.Sort([](const FTransform&, const FTransform&){ return FMath::FRand() < 0.5f; });
		All.SetNum(MaxPointsCount, false);
	}
	return All;
}