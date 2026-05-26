// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelDefinitions.h"

struct FVoxelMarchingMesherSettings
{
	int32 ChunkSizeX = 0;
	int32 ChunkSizeY = 0;
	int32 ChunkSizeZ = 0;
	float VoxelSize = 100.f;
	TFunctionRef<FVoxelData(int32 X, int32 Y, int32 Z)> GetSample;
};

class ONETWOSHOOT_API FVoxelMarchingMesher
{
public:
	static void Generate(const FVoxelMarchingMesherSettings& Settings, TArray<FChunkMeshData>& MeshSections);

private:
	static void MarchCube(const FVoxelMarchingMesherSettings& Settings, TArray<FChunkMeshData>& MeshSections, int32 X, int32 Y, int32 Z, const FVoxelData Cube[8]);
	static EVoxelBlockType GetDominantBlockType(const FVoxelData Cube[8]);
	static bool ContainsBlockType(const FVoxelData Cube[8], EVoxelBlockType BlockType);
	static void AddTriangle(TArray<FChunkMeshData>& MeshSections, const FVector& V1, const FVector& V2, const FVector& V3, EVoxelBlockType BlockType, const FVoxelData Cube[8], const int32 TriangleOrder[3]);
};
