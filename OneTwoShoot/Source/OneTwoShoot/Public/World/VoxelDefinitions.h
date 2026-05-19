// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "VoxelDefinitions.generated.h"

UENUM(BlueprintType)
enum class EVoxelBlockType : uint8
{
	Air,
	Grass,
	Dirt,
	Stone,
	Steel,
	Glass
};

UENUM(BlueprintType)
enum class EVoxelRenderMode : uint8
{
	Blocky,
	Marching
};

UENUM(BlueprintType)
enum class EVoxelTerrainMaterialSection : uint8
{
	Grass,
	Dirt,
	Stone,
	Steel,
	Glass,
	Count UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FVoxelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
	EVoxelBlockType BlockType = EVoxelBlockType::Air;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
	float Density = 1.0f; // 현재 fixed-center Marching에서는 사용하지 않는 legacy 값이다.
};

UENUM(BlueprintType)
enum class EVoxelDirection : uint8
{
	Forward,
	Right,
	Back,
	Left,
	Up,
	Down
};

USTRUCT()
struct FChunkMeshData
{
	GENERATED_BODY()

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	void Reset()
	{
		Vertices.Reset();
		Triangles.Reset();
		Normals.Reset();
		UVs.Reset();
		VertexColors.Reset();
		Tangents.Reset();
	}

	void Reserve(int32 VertexCount, int32 TriangleIndexCount)
	{
		Vertices.Reserve(VertexCount);
		Triangles.Reserve(TriangleIndexCount);
		Normals.Reserve(VertexCount);
		UVs.Reserve(VertexCount);
		VertexColors.Reserve(VertexCount);
		Tangents.Reserve(VertexCount);
	}
};

FORCEINLINE int32 GetVoxelTerrainMaterialSectionCount()
{
	return static_cast<int32>(EVoxelTerrainMaterialSection::Count);
}

FORCEINLINE int32 GetVoxelTerrainMaterialSectionIndex(EVoxelBlockType BlockType, EVoxelDirection Direction)
{
	switch (BlockType)
	{
	case EVoxelBlockType::Grass:
		return Direction == EVoxelDirection::Up
			? static_cast<int32>(EVoxelTerrainMaterialSection::Grass)
			: static_cast<int32>(EVoxelTerrainMaterialSection::Dirt);
	case EVoxelBlockType::Dirt:
		return static_cast<int32>(EVoxelTerrainMaterialSection::Dirt);
	case EVoxelBlockType::Stone:
		return static_cast<int32>(EVoxelTerrainMaterialSection::Stone);
	case EVoxelBlockType::Steel:
		return static_cast<int32>(EVoxelTerrainMaterialSection::Steel);
	case EVoxelBlockType::Glass:
		return static_cast<int32>(EVoxelTerrainMaterialSection::Glass);
	default:
		return INDEX_NONE;
	}
}

FORCEINLINE int32 GetVoxelTerrainMaterialSectionIndex(EVoxelBlockType BlockType, const FVector& Normal)
{
	switch (BlockType)
	{
	case EVoxelBlockType::Grass:
		return Normal.Z >= 0.55f
			? static_cast<int32>(EVoxelTerrainMaterialSection::Grass)
			: static_cast<int32>(EVoxelTerrainMaterialSection::Dirt);
	case EVoxelBlockType::Dirt:
		return static_cast<int32>(EVoxelTerrainMaterialSection::Dirt);
	case EVoxelBlockType::Stone:
		return static_cast<int32>(EVoxelTerrainMaterialSection::Stone);
	case EVoxelBlockType::Steel:
		return static_cast<int32>(EVoxelTerrainMaterialSection::Steel);
	case EVoxelBlockType::Glass:
		return static_cast<int32>(EVoxelTerrainMaterialSection::Glass);
	default:
		return INDEX_NONE;
	}
}
