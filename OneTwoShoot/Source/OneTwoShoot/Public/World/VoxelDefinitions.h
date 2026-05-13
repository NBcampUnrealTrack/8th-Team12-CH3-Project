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
