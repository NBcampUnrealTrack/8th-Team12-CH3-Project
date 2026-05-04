// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

#include "VoxelChunkActor.generated.h"


class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class ONETWOSHOOT_API AVoxelChunkActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVoxelChunkActor();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Debug")
	void DestroyVoxelsAtWorldLocation(FVector WorldLocation, float Radius);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Debug")
	void DebugDestroyCenter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug")
	float DebugDestroyRadius = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug")
	FVector DebugDestroyLocalOffset = FVector::ZeroVector;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(CallInEditor, Category = "Voxel") // Editor-Details-Voxel. Rebuild the Mesh for the chunk.
	void RebuildChunk();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
	int32 ChunkSizeX = 16;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
	int32 ChunkSizeY = 16;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
	int32 ChunkSizeZ = 16;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
	float VoxelSize = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	UProceduralMeshComponent* Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel|Material")
	UMaterialInterface* VoxelMaterial;

	UPROPERTY()
	TArray<bool> Voxels;


	void InitializeVoxels();

	int32 GetVoxelIndex(int32 X, int32 Y, int32 Z) const;

	bool IsCoordinateValid(int32 X, int32 Y, int32 Z) const;

	bool IsVoxelSolid(int32 X, int32 Y, int32 Z) const;

	void GenerateMesh();

private:
	
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
	void AddCube(int32 X, int32 Y, int32 Z);
	void AddFace(
		const FVector& V0,
		const FVector& V1,
		const FVector& V2,
		const FVector& V3,
		const FVector& Normal
	);
};
