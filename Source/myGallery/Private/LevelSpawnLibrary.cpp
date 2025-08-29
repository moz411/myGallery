// LevelSpawnLibrary.cpp
#include "LevelSpawnLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"

static ULevelStreaming* FindStreamingLevel(UWorld* World, FName LevelName)
{
	if (!World) return nullptr;

	// 1) Tentative directe (BP/UGameplayStatics attend soit le short name, soit le chemin package)
	if (ULevelStreaming* LS = UGameplayStatics::GetStreamingLevel(World, LevelName))
	{
		return LS;
	}

	// 2) Fallback : comparer le nom de package de l’asset (utile si LevelName ≠ short name exact)
	for (ULevelStreaming* LS : World->GetStreamingLevels())
	{
		if (!LS) continue;

#if ENGINE_MAJOR_VERSION >= 5
		const FName Pkg = LS->GetWorldAssetPackageFName(); // UE5
#else
		const FName Pkg = FName(*LS->GetWorldAssetPackageName()); // UE4
#endif
		if (Pkg == LevelName) return LS;

		// Autoriser le short name sans chemin
		const FString Short = FPackageName::GetShortName(Pkg);
		if (LevelName == FName(*Short))
		{
			return LS;
		}
	}
	return nullptr;
}

void ULevelSpawnLibrary::EnsureStreamingLevelLoaded(UObject* WorldContextObject,
	FName StreamingLevelName,
	bool bMakeVisibleAfterLoad,
	FLatentActionInfo LatentInfo)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);

	// Déclenche le chargement latent (équivaut au node BP LoadStreamLevel)
	UGameplayStatics::LoadStreamLevel(World,
		StreamingLevelName,
		/*bMakeVisibleAfterLoad=*/bMakeVisibleAfterLoad,
		/*bShouldBlockOnLoad=*/false,
		LatentInfo);
}

AActor* ULevelSpawnLibrary::SpawnActorInStreamingLevel(UObject* WorldContextObject,
	TSubclassOf<AActor> ActorClass,
	const FTransform& Transform,
	FName StreamingLevelName,
	ESpawnActorCollisionHandlingMethod CollisionHandling)
{
	if (!ActorClass) return nullptr;

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	ULevelStreaming* LS = FindStreamingLevel(World, StreamingLevelName);
	if (!LS || !LS->IsLevelLoaded())
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnActorInStreamingLevel: level '%s' introuvable ou non chargé."), *StreamingLevelName.ToString());
		return nullptr;
	}

	ULevel* TargetLevel = LS->GetLoadedLevel();
	if (!TargetLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnActorInStreamingLevel: LoadedLevel null pour '%s'."), *StreamingLevelName.ToString());
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.OverrideLevel = TargetLevel; // clé : spawn dans ce sublevel
	Params.SpawnCollisionHandlingOverride = CollisionHandling;

	return World->SpawnActor<AActor>(*ActorClass, Transform, Params);
}
