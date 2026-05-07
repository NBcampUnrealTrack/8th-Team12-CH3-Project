// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorld.generated.h"

class AVoxelChunkActor;
class ABaseProjectile;
enum class EVoxelBlockType : uint8;

UCLASS()
class ONETWOSHOOT_API AVoxelWorld : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVoxelWorld();

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void RefreshChunkList();

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void RegisterChunk(AVoxelChunkActor* Chunk);

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void UnregisterChunk(AVoxelChunkActor* Chunk);

	UFUNCTION(BlueprintCallable, Category = "Voxel|World|Debug")
	void DebugDestroyAllChunks();

	UFUNCTION(BlueprintCallable, Category = "Voxel|World|Debug")
	void DebugDestroyRandomChunk();

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void DestroyVoxelsAtWorldLocation(FVector WorldLocation, float Radius);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|World|Debug")
	float DebugDestroyRadius = 200.f;

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void HandleProjectileExplosion(FVector HitLocation, float ExplosionRadius);

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void BindExistingProjectiles();


	//테스트용 추가 5종
	UFUNCTION(BlueprintCallable, Category = "Voxel|AI")
	bool IsWalkable(FIntVector GlobalCoords);

	UFUNCTION(BlueprintCallable, Category = "Voxel|AI")
	EVoxelBlockType GetVoxelTypeAt(FIntVector GlobalCoords);

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	FIntVector WorldToVoxelCoords(FVector WorldLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	FVector VoxelToWorldLocation(FIntVector VoxelCoords) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float VoxelSize = 100.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel|World")
	TArray<TObjectPtr<AVoxelChunkActor>> Chunks;

};
