// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelDefinitions.h"

struct FVoxelBlockyMesherSettings
{
	int32 ChunkSizeX = 0;
	int32 ChunkSizeY = 0;
	int32 ChunkSizeZ = 0;
	float VoxelSize = 100.f;
	TFunctionRef<bool(int32 X, int32 Y, int32 Z)> IsVoxelSolid;
};

class ONETWOSHOOT_API FVoxelBlockyMesher
{
public:
	static void Generate(const FVoxelBlockyMesherSettings& Settings, FChunkMeshData& MeshData);

private:
	static void AddCube(const FVoxelBlockyMesherSettings& Settings, FChunkMeshData& MeshData, int32 X, int32 Y, int32 Z);
	static void AddFace(const FVoxelBlockyMesherSettings& Settings, FChunkMeshData& MeshData, EVoxelDirection Direction, const FVector& Base);
	static FIntVector GetDirectionOffset(EVoxelDirection Direction);
	static FVector GetDirectionNormal(EVoxelDirection Direction);
	static TArray<FVector> GetFaceVertices(EVoxelDirection Direction, const FVector& Base, float VoxelSize);
};
