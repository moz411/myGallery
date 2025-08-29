// LevelSpawnLibrary.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/LevelStreaming.h"
#include "LevelSpawnLibrary.generated.h"

UCLASS()
class ULevelSpawnLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Assure (de façon latente) que le sublevel est chargé (optionnel : visible ou non).
	UFUNCTION(BlueprintCallable, Category="myGallery", meta=(WorldContext="WorldContextObject", Latent, LatentInfo="LatentInfo"))
	static void EnsureStreamingLevelLoaded(UObject* WorldContextObject,
		FName StreamingLevelName,
		bool bMakeVisibleAfterLoad,
		FLatentActionInfo LatentInfo);

	// Spawn directement DANS un sublevel déjà chargé (OverrideLevel).
	UFUNCTION(BlueprintCallable, Category="myGallery", meta=(WorldContext="WorldContextObject"))
	static AActor* SpawnActorInStreamingLevel(UObject* WorldContextObject,
		TSubclassOf<AActor> ActorClass,
		const FTransform& Transform,
		FName StreamingLevelName,
		ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

};
