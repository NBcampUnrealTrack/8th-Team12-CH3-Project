// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/World/VoxelBlockyMesher.h"

void FVoxelBlockyMesher::Generate(const FVoxelBlockyMesherSettings& Settings, FChunkMeshData& MeshData)
{
	MeshData.Reset();
	MeshData.Reserve(50000, 75000);

	for (int32 Z = 0; Z < Settings.ChunkSizeZ; Z++)
	{
		for (int32 Y = 0; Y < Settings.ChunkSizeY; Y++)
		{
			for (int32 X = 0; X < Settings.ChunkSizeX; X++)
			{
				if (Settings.IsVoxelSolid(X, Y, Z))
				{
					AddCube(Settings, MeshData, X, Y, Z);
				}
			}
		}
	}
}

void FVoxelBlockyMesher::AddCube(const FVoxelBlockyMesherSettings& Settings, FChunkMeshData& MeshData, int32 X, int32 Y, int32 Z)
{
	const FVector Base = FVector(X, Y, Z) * Settings.VoxelSize;

	for (const EVoxelDirection Direction : {
		EVoxelDirection::Forward,
		EVoxelDirection::Right,
		EVoxelDirection::Back,
		EVoxelDirection::Left,
		EVoxelDirection::Up,
		EVoxelDirection::Down
	})
	{
		const FIntVector Offset = GetDirectionOffset(Direction);

		if (!Settings.IsVoxelSolid(X + Offset.X, Y + Offset.Y, Z + Offset.Z))
		{
			AddFace(Settings, MeshData, Direction, Base);
		}
	}
}

void FVoxelBlockyMesher::AddFace(const FVoxelBlockyMesherSettings& Settings, FChunkMeshData& MeshData, EVoxelDirection Direction, const FVector& Base)
{
	const int32 StartIndex = MeshData.Vertices.Num();
	const TArray<FVector> FaceVertices = GetFaceVertices(Direction, Base, Settings.VoxelSize);
	const FVector Normal = GetDirectionNormal(Direction);

	MeshData.Vertices.Append(FaceVertices);

	MeshData.Triangles.Add(StartIndex + 0);
	MeshData.Triangles.Add(StartIndex + 2);
	MeshData.Triangles.Add(StartIndex + 1);

	MeshData.Triangles.Add(StartIndex + 0);
	MeshData.Triangles.Add(StartIndex + 3);
	MeshData.Triangles.Add(StartIndex + 2);

	MeshData.Normals.Append({ Normal, Normal, Normal, Normal });

	MeshData.UVs.Append({
		FVector2D(0.f, 0.f),
		FVector2D(1.f, 0.f),
		FVector2D(1.f, 1.f),
		FVector2D(0.f, 1.f)
	});

	MeshData.VertexColors.Append({
		FLinearColor::White,
		FLinearColor::White,
		FLinearColor::White,
		FLinearColor::White
	});

	const FVector TangentX = (FaceVertices[1] - FaceVertices[0]).GetSafeNormal();
	const FProcMeshTangent Tangent(TangentX, false);
	MeshData.Tangents.Append({ Tangent, Tangent, Tangent, Tangent });
}

FIntVector FVoxelBlockyMesher::GetDirectionOffset(EVoxelDirection Direction)
{
	switch (Direction)
	{
	case EVoxelDirection::Forward:
		return FIntVector(1, 0, 0);
	case EVoxelDirection::Right:
		return FIntVector(0, 1, 0);
	case EVoxelDirection::Back:
		return FIntVector(-1, 0, 0);
	case EVoxelDirection::Left:
		return FIntVector(0, -1, 0);
	case EVoxelDirection::Up:
		return FIntVector(0, 0, 1);
	case EVoxelDirection::Down:
		return FIntVector(0, 0, -1);
	}

	checkNoEntry();
	return FIntVector::ZeroValue;
}

FVector FVoxelBlockyMesher::GetDirectionNormal(EVoxelDirection Direction)
{
	return FVector(GetDirectionOffset(Direction));
}

TArray<FVector> FVoxelBlockyMesher::GetFaceVertices(EVoxelDirection Direction, const FVector& Base, float VoxelSize)
{
	constexpr int32 FaceVertexCount = 4;
	constexpr int32 FaceVertexIndices[6][FaceVertexCount] = {
		{ 1, 3, 7, 5 },
		{ 2, 6, 7, 3 },
		{ 0, 4, 6, 2 },
		{ 0, 1, 5, 4 },
		{ 4, 5, 7, 6 },
		{ 0, 2, 3, 1 }
	};

	const float S = VoxelSize;
	const FVector BlockVertexData[8] = {
		Base,
		Base + FVector(S, 0.f, 0.f),
		Base + FVector(0.f, S, 0.f),
		Base + FVector(S, S, 0.f),
		Base + FVector(0.f, 0.f, S),
		Base + FVector(S, 0.f, S),
		Base + FVector(0.f, S, S),
		Base + FVector(S, S, S)
	};

	const int32 DirectionIndex = static_cast<int32>(Direction);
	check(DirectionIndex >= 0 && DirectionIndex < UE_ARRAY_COUNT(FaceVertexIndices));

	TArray<FVector> FaceVertices;
	FaceVertices.Reserve(FaceVertexCount);

	for (int32 VertexIndex = 0; VertexIndex < FaceVertexCount; VertexIndex++)
	{
		FaceVertices.Add(BlockVertexData[FaceVertexIndices[DirectionIndex][VertexIndex]]);
	}

	return FaceVertices;
}
