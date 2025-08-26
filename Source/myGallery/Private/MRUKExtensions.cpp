#include "MRUKExtensions.h"
#include "MathUtil.h"
#include "MRUtilityKitAnchor.h"
#include "MRUtilityKitSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Containers/Set.h"

static FQuat MakeQuatWithZAlignedToNormal(const FVector& R, const FVector& U, bool bFlipNormal=false)
{
	// N = R x U (sens MRUK). Optionnellement inverser si on veut orienter vers l'intérieur.
	FVector N = FVector::CrossProduct(R, U).GetSafeNormal();
	if (bFlipNormal) N *= -1.f;

	// Tangent préféré = R (mais doit être non colinéaire à N)
	FVector X = R - FVector::DotProduct(R, N) * N; // projeter R dans le plan tangent
	if (!X.Normalize())
	{
		// Si R~//N, fallback sur U
		X = U - FVector::DotProduct(U, N) * N;
		X.Normalize(); // plan MRUK bien formé -> doit réussir
	}

	const FVector Y = FVector::CrossProduct(N, X); // base droitière (X,Y,Z=N)
	const FMatrix M(FMatrix::Identity);
	FMatrix Basis(
		FPlane(X, 0),  // X -> 1ère colonne
		FPlane(Y, 0),  // Y -> 2ème
		FPlane(N, 0),  // Z -> 3ème
		FPlane(0,0,0,1)
	);
	return Basis.ToQuat();
}

// --- GeneratePoints -> retourne des FTransform (Loc=P, Rot=Z=Normal du plan) ---
TArray<FTransform> GeneratePoints(const FTransform& Plane, const FBox2D& PlaneBounds,
    double PointsPerUnitX, double PointsPerUnitY, double WorldToMeters)
{
	const FQuat   Q = Plane.GetRotation();
	const FVector R = Q.GetRightVector();      // axe X du plan MRUK
	const FVector U = Q.GetUpVector();         // axe Y du plan MRUK

	const FVector2D Size = PlaneBounds.GetSize();
	const int32 NX = FMath::Max(1, FMath::CeilToInt(PointsPerUnitX * (Size.X / WorldToMeters)) - 1);
	const int32 NY = FMath::Max(1, FMath::CeilToInt(PointsPerUnitY * (Size.Y / WorldToMeters)) - 1);
	const FVector2D Stride(Size.X / (NX + 1), Size.Y / (NY + 1));

	const FVector BL = Plane.GetLocation() - R * (Size.X * 0.5f) - U * (Size.Y * 0.5f);
	const FQuat Qplane = MakeQuatWithZAlignedToNormal(R, U ,false);

	TArray<FTransform> Xforms;
	Xforms.Reserve(NX * NY);

	for (int32 j=0; j<NY; ++j)
	for (int32 i=0; i<NX; ++i)
	{
		const float dx = (i + 1) * Stride.X;
		const float dy = (j + 1) * Stride.Y;
		const FVector P = BL + dx * R + dy * U;       // point sur le plan MRUK
		Xforms.Emplace(Qplane, P, FVector(.1f));      // Z local = normale du plan
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
		                          PointsPerUnitX, PointsPerUnitY, W2M));
	}

	if (MaxPointsCount > 0 && All.Num() > MaxPointsCount)
	{
		All.Sort([](const FTransform&, const FTransform&){ return FMath::FRand() < 0.5f; });
		All.SetNum(MaxPointsCount, false);
	}
	return All;
}