#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MRUKExtensions.generated.h"

class AMRUKRoom;
class AMRUKAnchor;

UCLASS()
class UMRUKExtensions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** Variante “surface only” de ComputeRoomBoxGrid :
	 * - Murs + Plafond + Sol
	 * - Pas de plans intermédiaires
	 * - Dédoublonnage cross-planes (coins/arêtes)
	 *
	 * PointsPerUnitX/Y : densité par mètre dans les axes locaux du plan MRUK.
	 * MergeToleranceFactor : fraction du pas local pour fusionner les doublons (0.2–0.6).
	 */
	UFUNCTION(BlueprintCallable, Category="myGallery|MRUtilityKit|Grid")
	static TArray<FVector> ComputeRoomBoxGridSurfaceOnly(
	const AMRUKRoom* Room,
	int32 MaxPointsCount = 1000,
	double PointsPerUnitX = 1.0,
	double PointsPerUnitY = 1.0
	);
};
