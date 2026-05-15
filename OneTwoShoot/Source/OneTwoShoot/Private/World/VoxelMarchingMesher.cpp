// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/World/VoxelMarchingMesher.h"
#include "../Public/World/VoxelMarchingCubesTables.h"

void FVoxelMarchingMesher::Generate(const FVoxelMarchingMesherSettings& Settings, FChunkMeshData& MeshData)
{
	MeshData.Reset();
	MeshData.Reserve(50000, 75000);

	FVoxelData Cube[8];

	for (int32 Z = -1; Z < Settings.ChunkSizeZ; Z++)
	{
		for (int32 Y = -1; Y < Settings.ChunkSizeY; Y++)
		{
			for (int32 X = -1; X < Settings.ChunkSizeX; X++)
			{
				for (int32 CornerIndex = 0; CornerIndex < 8; CornerIndex++)
				{
					Cube[CornerIndex] = Settings.GetSample(
						X + VoxelMarchingCubes::VertexOffset[CornerIndex][0],
						Y + VoxelMarchingCubes::VertexOffset[CornerIndex][1],
						Z + VoxelMarchingCubes::VertexOffset[CornerIndex][2]
					);
				}

				MarchCube(Settings, MeshData, X, Y, Z, Cube);
			}
		}
	}
}

void FVoxelMarchingMesher::MarchCube(const FVoxelMarchingMesherSettings& Settings, FChunkMeshData& MeshData, int32 X, int32 Y, int32 Z, const FVoxelData Cube[8])
{
	int32 VertexMask = 0;
	FVector EdgeVertex[12];

	for (int32 CornerIndex = 0; CornerIndex < 8; CornerIndex++)
	{
		if (Cube[CornerIndex].BlockType != EVoxelBlockType::Air)
		{
			VertexMask |= 1 << CornerIndex;
		}
	}

	const int32 EdgeMask = VoxelMarchingCubes::CubeEdgeFlags[VertexMask];
	if (EdgeMask == 0)
	{
		return;
	}

	for (int32 EdgeIndex = 0; EdgeIndex < 12; EdgeIndex++)
	{
		if ((EdgeMask & (1 << EdgeIndex)) == 0)
		{
			continue;
		}

		const int32 CornerA = VoxelMarchingCubes::EdgeConnection[EdgeIndex][0];
		const int32 CornerB = VoxelMarchingCubes::EdgeConnection[EdgeIndex][1];
		const float Offset = 0.5f;

		EdgeVertex[EdgeIndex].X = X + (VoxelMarchingCubes::VertexOffset[CornerA][0] + Offset * VoxelMarchingCubes::EdgeDirection[EdgeIndex][0]);
		EdgeVertex[EdgeIndex].Y = Y + (VoxelMarchingCubes::VertexOffset[CornerA][1] + Offset * VoxelMarchingCubes::EdgeDirection[EdgeIndex][1]);
		EdgeVertex[EdgeIndex].Z = Z + (VoxelMarchingCubes::VertexOffset[CornerA][2] + Offset * VoxelMarchingCubes::EdgeDirection[EdgeIndex][2]);
		EdgeVertex[EdgeIndex] *= Settings.VoxelSize;
	}

	const int32 TriangleOrder[3] = { 0, 1, 2 };
	const EVoxelBlockType BlockType = GetDominantBlockType(Cube);

	for (int32 TriangleIndex = 0; TriangleIndex < 5; TriangleIndex++)
	{
		const int32 EdgeA = VoxelMarchingCubes::TriangleConnectionTable[VertexMask][TriangleIndex * 3];
		if (EdgeA < 0)
		{
			break;
		}

		const int32 EdgeB = VoxelMarchingCubes::TriangleConnectionTable[VertexMask][TriangleIndex * 3 + 1];
		const int32 EdgeC = VoxelMarchingCubes::TriangleConnectionTable[VertexMask][TriangleIndex * 3 + 2];

		AddTriangle(MeshData, EdgeVertex[EdgeA], EdgeVertex[EdgeB], EdgeVertex[EdgeC], BlockType, TriangleOrder);
	}
}

EVoxelBlockType FVoxelMarchingMesher::GetDominantBlockType(const FVoxelData Cube[8])
{
	for (int32 CornerIndex = 0; CornerIndex < 8; CornerIndex++)
	{
		if (Cube[CornerIndex].BlockType != EVoxelBlockType::Air)
		{
			return Cube[CornerIndex].BlockType;
		}
	}

	return EVoxelBlockType::Dirt;
}

void FVoxelMarchingMesher::AddTriangle(FChunkMeshData& MeshData, const FVector& V1, const FVector& V2, const FVector& V3, EVoxelBlockType BlockType, const int32 TriangleOrder[3])
{
	const int32 StartIndex = MeshData.Vertices.Num();
	const FVector TriangleVertices[3] = { V1, V2, V3 };
	FVector Normal = -FVector::CrossProduct(
		TriangleVertices[TriangleOrder[1]] - TriangleVertices[TriangleOrder[0]],
		TriangleVertices[TriangleOrder[2]] - TriangleVertices[TriangleOrder[0]]
	).GetSafeNormal();
	if (Normal.IsNearlyZero())
	{
		Normal = FVector::UpVector;
	}
	const FLinearColor VertexColor = BlockType == EVoxelBlockType::Grass ? FLinearColor::Green : FLinearColor::White;

	MeshData.Vertices.Append({ V1, V2, V3 });
	MeshData.Triangles.Append({
		StartIndex + TriangleOrder[0],
		StartIndex + TriangleOrder[1],
		StartIndex + TriangleOrder[2]
	});
	MeshData.Normals.Append({ Normal, Normal, Normal });
	MeshData.UVs.Append({
		FVector2D(0.f, 0.f),
		FVector2D(1.f, 0.f),
		FVector2D(0.f, 1.f)
	});
	MeshData.VertexColors.Append({
		VertexColor,
		VertexColor,
		VertexColor
	});

	const FVector TangentX = (TriangleVertices[TriangleOrder[1]] - TriangleVertices[TriangleOrder[0]]).GetSafeNormal();
	const FProcMeshTangent Tangent(TangentX.IsNearlyZero() ? FVector::ForwardVector : TangentX, false);
	MeshData.Tangents.Append({ Tangent, Tangent, Tangent });
}
