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
	TFunctionRef<FVoxelData(int32 X, int32 Y, int32 Z)> GetVoxelData;
};

class ONETWOSHOOT_API FVoxelBlockyMesher
{
public:
	static void Generate(const FVoxelBlockyMesherSettings& Settings, TArray<FChunkMeshData>& MeshSections);

private:
	static void AddCube(const FVoxelBlockyMesherSettings& Settings, TArray<FChunkMeshData>& MeshSections, int32 X, int32 Y, int32 Z, EVoxelBlockType BlockType);
	static void AddFace(const FVoxelBlockyMesherSettings& Settings, TArray<FChunkMeshData>& MeshSections, EVoxelDirection Direction, const FVector& Base, EVoxelBlockType BlockType);
	static FIntVector GetDirectionOffset(EVoxelDirection Direction);
	static FVector GetDirectionNormal(EVoxelDirection Direction);
	static TArray<FVector> GetFaceVertices(EVoxelDirection Direction, const FVector& Base, float VoxelSize);
};
