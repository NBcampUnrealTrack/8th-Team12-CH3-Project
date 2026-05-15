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
	EVoxelRenderMode RenderMode = EVoxelRenderMode::MarchingCenter;
	float SurfaceLevel = 0.0f;
	int32 SteppedInterpolationSteps = 4;
	TFunctionRef<FVoxelData(int32 X, int32 Y, int32 Z)> GetSample;
};

class ONETWOSHOOT_API FVoxelMarchingMesher
{
public:
	static void Generate(const FVoxelMarchingMesherSettings& Settings, FChunkMeshData& MeshData);

private:
	static void MarchCube(const FVoxelMarchingMesherSettings& Settings, FChunkMeshData& MeshData, int32 X, int32 Y, int32 Z, const FVoxelData Cube[8]);
	static float GetInterpolationOffset(const FVoxelMarchingMesherSettings& Settings, float DensityA, float DensityB);
	static float QuantizeOffset(const FVoxelMarchingMesherSettings& Settings, float Offset);
	static EVoxelBlockType GetDominantBlockType(const FVoxelMarchingMesherSettings& Settings, const FVoxelData Cube[8]);
	static void AddTriangle(FChunkMeshData& MeshData, const FVector& V1, const FVector& V2, const FVector& V3, EVoxelBlockType BlockType, const int32 TriangleOrder[3]);
};
