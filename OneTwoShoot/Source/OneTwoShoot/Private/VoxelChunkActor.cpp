#include "VoxelChunkActor.h"

AVoxelChunkActor::AVoxelChunkActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = Mesh;
	Mesh->bUseAsyncCooking = true;
}




void AVoxelChunkActor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("VoxelChunk Initialized: %d voxels"), Voxels.Num());
	
	//for (int32 Z = 0; Z < ChunkSizeZ; Z++) //-> 각 Voxel마다 로그찍기
	//{
	//	for (int32 Y = 0; Y < ChunkSizeY; Y++)
	//	{
	//		for (int32 X = 0; X < ChunkSizeX; X++)
	//		{
	//		{
	//			GetVoxelIndex(X, Y, Z);
	//		}
	//	}
	//}
	RebuildChunk();
}



void AVoxelChunkActor::InitializeVoxels()
{
	const int32 TotalVoxelCount = ChunkSizeX * ChunkSizeY * ChunkSizeZ;

	Voxels.SetNum(TotalVoxelCount);

	for (int32 i = 0; i < Voxels.Num(); i++)
	{
		Voxels[i] = EVoxelBlockType::Stone;
	}
}

int32 AVoxelChunkActor::GetVoxelIndex(int32 X, int32 Y, int32 Z) const
{
	const int32 Index = X + (Y * ChunkSizeX) + (Z * ChunkSizeX * ChunkSizeY);
	//UE_LOG(LogTemp,Warning,TEXT("VoxelIndex: X=%d Y=%d Z=%d | Dec=%d Hex=0x%X"),X, Y, Z, Index, Index);
	return Index;	
}

bool AVoxelChunkActor::IsCoordinateValid(int32 X, int32 Y, int32 Z) const
{
	return X >= 0 && X < ChunkSizeX &&
		Y >= 0 && Y < ChunkSizeY &&
		Z >= 0 && Z < ChunkSizeZ;
}

bool AVoxelChunkActor::IsVoxelSolid(int32 X, int32 Y, int32 Z) const
{
	if (!IsCoordinateValid(X, Y, Z))
	{
		return false;
	}

	return Voxels[GetVoxelIndex(X, Y, Z)] != EVoxelBlockType::Air;
}

void AVoxelChunkActor::GenerateMesh()
{
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_GenerateMesh);
{
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_ClearArrays);
	MeshData.Reset();
}
{
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_ReserveArrays);
	MeshData.Reserve(50000, 75000);
}

{
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_LoopAndAddFaces);

	for (int32 Z = 0; Z < ChunkSizeZ; Z++)
	{
		for (int32 Y = 0; Y < ChunkSizeY; Y++)
		{
			for (int32 X = 0; X < ChunkSizeX; X++)
			{
				if (IsVoxelSolid(X, Y, Z))
				{
					AddCube(X, Y, Z);
				}
			}
		}
	}
}
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_ClearMeshSection);

	Mesh->ClearAllMeshSections();
	
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_CreateMeshSection);

	Mesh->CreateMeshSection_LinearColor(
		0,
		MeshData.Vertices,
		MeshData.Triangles,
		MeshData.Normals,
		MeshData.UVs,
		MeshData.VertexColors,
		MeshData.Tangents,
		true
	);
	if (VoxelMaterial)
	{
		Mesh->SetMaterial(0, VoxelMaterial);
	}

	UE_LOG(LogTemp, Warning, TEXT("Generated Mesh: Vertices=%d / Triangles=%d"), MeshData.Vertices.Num(), MeshData.Triangles.Num() / 3);
	UE_LOG(LogTemp, Warning, TEXT("Material: %s"),
		VoxelMaterial ? *VoxelMaterial->GetName() : TEXT("None"));
}

void AVoxelChunkActor::RebuildChunk()
{
	if (!Mesh) return;

	InitializeVoxels();
	GenerateMesh();
}

void AVoxelChunkActor::AddCube(int32 X, int32 Y, int32 Z)
{
	const FVector Base = FVector(X, Y, Z) * VoxelSize;

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

		if (!IsVoxelSolid(X + Offset.X, Y + Offset.Y, Z + Offset.Z))
		{
			AddFace(Direction, Base);
		}
	}
}

void AVoxelChunkActor::AddFace(EVoxelDirection Direction, const FVector& Base)
{
	const int32 StartIndex = MeshData.Vertices.Num();
	const TArray<FVector> FaceVertices = GetFaceVertices(Direction, Base);
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

FIntVector AVoxelChunkActor::GetDirectionOffset(EVoxelDirection Direction) const //FIntVector 형태 방향 벡터. 1*1*1 그리드 관리용.
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
	// 존재하는 모든 방향 switch 처리 - 기하법칙이라 switch로 두는 게 맞을듯?
	checkNoEntry();//코드가 여기까지 내려오면 터짐
	return FIntVector::ZeroValue; // 물리적으로 존재하지 않는 방향 들어오면 000 반환
}

FVector AVoxelChunkActor::GetDirectionNormal(EVoxelDirection Direction) const //FIntVector 형태인 GetDirectionOffset를 FVector로 변환. Procedural Mesh에서 Normal/Tangent용
{
	return FVector(GetDirectionOffset(Direction));
}

TArray<FVector> AVoxelChunkActor::GetFaceVertices(EVoxelDirection Direction, const FVector& Base) const
{
	constexpr int32 FaceVertexCount = 4;
	constexpr int32 FaceVertexIndices[6][FaceVertexCount] = {
		{ 1, 3, 7, 5 }, // Forward
		{ 2, 6, 7, 3 }, // Right
		{ 0, 4, 6, 2 }, // Back
		{ 0, 1, 5, 4 }, // Left
		{ 4, 5, 7, 6 }, // Up
		{ 0, 2, 3, 1 }  // Down
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
void AVoxelChunkActor::DestroyVoxelsAtWorldLocation(FVector WorldLocation, float Radius)
{
	const FVector LocalLocation = GetActorTransform().InverseTransformPosition(WorldLocation);

	const int32 CenterX = FMath::FloorToInt(LocalLocation.X / VoxelSize);
	const int32 CenterY = FMath::FloorToInt(LocalLocation.Y / VoxelSize);
	const int32 CenterZ = FMath::FloorToInt(LocalLocation.Z / VoxelSize);

	const int32 RadiusInVoxels = FMath::CeilToInt(Radius / VoxelSize);

	for (int32 Z = CenterZ - RadiusInVoxels; Z <= CenterZ + RadiusInVoxels; Z++)
	{
		for (int32 Y = CenterY - RadiusInVoxels; Y <= CenterY + RadiusInVoxels; Y++)
		{
			for (int32 X = CenterX - RadiusInVoxels; X <= CenterX + RadiusInVoxels; X++)
			{
				if (!IsCoordinateValid(X, Y, Z)) continue;

				const FVector VoxelCenter = FVector(
					(X + 0.5f) * VoxelSize,
					(Y + 0.5f) * VoxelSize,
					(Z + 0.5f) * VoxelSize
				);

				if (FVector::Dist(VoxelCenter, LocalLocation) <= Radius)
				{
					Voxels[GetVoxelIndex(X, Y, Z)] = EVoxelBlockType::Air;
					
				}
			}
		}
	}
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_GenerateMesh);

	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Generate Mesh Called."));
	GenerateMesh();

	//BP_OnDebugDestroyed(WorldLocation, Radius);
}

void AVoxelChunkActor::DebugDestroyCenter()
{
	UE_LOG(LogTemp, Warning, TEXT("Voxel Count Before Destroy: %d"), Voxels.Num());
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_DebugDestroyCenter);

	const FVector ChunkCenterLocal = FVector(
		ChunkSizeX * VoxelSize * 0.5f,
		ChunkSizeY * VoxelSize * 0.5f,
		ChunkSizeZ * VoxelSize * 0.5f
	);
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_FindDestroyPosition);

	const FVector RandomLocalOffset = FVector(
		FMath::FRandRange(-ChunkSizeX * VoxelSize * 0.5f, ChunkSizeX * VoxelSize * 0.5f),
		FMath::FRandRange(-ChunkSizeY * VoxelSize * 0.5f, ChunkSizeY * VoxelSize * 0.5f),
		FMath::FRandRange(-ChunkSizeZ * VoxelSize * 0.5f, ChunkSizeZ * VoxelSize * 0.5f)
	);
	const FVector TargetWorldLocation = GetActorTransform().TransformPosition(
		ChunkCenterLocal + RandomLocalOffset);
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_DestroyVoxels);

	DestroyVoxelsAtWorldLocation(TargetWorldLocation, DebugDestroyRadius);

	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_DebugDestroyCenter);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Debug Destroy Random Local Offset: %s / Radius: %.1f"),
		*DebugDestroyLocalOffset.ToString(),
		DebugDestroyRadius
	);
}

