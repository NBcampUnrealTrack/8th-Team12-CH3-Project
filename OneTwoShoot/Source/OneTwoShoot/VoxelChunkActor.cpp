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
		Voxels[i] = true;
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

	return Voxels[GetVoxelIndex(X, Y, Z)];
}

void AVoxelChunkActor::GenerateMesh()
{
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_GenerateMesh);
{
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_ClearArrays);
	Vertices.Reset();
	Triangles.Reset();
	Normals.Reset();
	UVs.Reset();
	VertexColors.Reset();
	Tangents.Reset();
}
{
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_ReserveArrays);
	Vertices.Reserve(50000);
	Triangles.Reserve(75000);
	Normals.Reserve(50000);
	UVs.Reserve(50000);
	Tangents.Reserve(50000);
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
	TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_ClearMeshSection);

	Mesh->ClearAllMeshSections();
	
	TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_CreateMeshSection);

	Mesh->CreateMeshSection_LinearColor(
		0,
		Vertices,
		Triangles,
		Normals,
		UVs,
		VertexColors,
		Tangents,
		false
	);
	if (VoxelMaterial)
	{
		Mesh->SetMaterial(0, VoxelMaterial);
	}

	UE_LOG(LogTemp, Warning, TEXT("Generated Mesh: Vertices=%d / Triangles=%d"), Vertices.Num(), Triangles.Num() / 3);
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
	//전체 Cube Mesh 만드는
	const FVector Base = FVector(X, Y, Z) * VoxelSize;
	const float S = VoxelSize;

	const FVector Point000 = Base;
	const FVector Point100 = Base + FVector(S, 0.f, 0.f);
	const FVector Point010 = Base + FVector(0.f, S, 0.f);
	const FVector Point110 = Base + FVector(S, S, 0.f);

	const FVector Point001 = Base + FVector(0.f, 0.f, S);
	const FVector Point101 = Base + FVector(S, 0.f, S);
	const FVector Point011 = Base + FVector(0.f, S, S);
	const FVector Point111 = Base + FVector(S, S, S);

	// +X면
	if (!IsVoxelSolid(X + 1, Y, Z))
	{
		AddFace(Point100, Point110, Point111, Point101, FVector(1.f, 0.f, 0.f));
	}
	// -X면
	if (!IsVoxelSolid(X-1, Y, Z)) {
		AddFace(Point000, Point001, Point011, Point010, FVector(-1.f, 0.f, 0.f));
	}
	// +Y면
	if (!IsVoxelSolid(X, Y + 1, Z)) {
	AddFace(Point010, Point011, Point111, Point110, FVector(0.f, 1.f, 0.f));
	}
	// -Y면
	if (!IsVoxelSolid(X, Y - 1, Z)) {
	AddFace(Point000, Point100, Point101, Point001, FVector(0.f, -1.f, 0.f));
	}
	// +Z면
	if (!IsVoxelSolid(X, Y, Z+1)) {
	AddFace(Point001, Point101, Point111, Point011, FVector(0.f, 0.f, 1.f));
	}
	// -Z면
	if (!IsVoxelSolid(X, Y, Z-1)) {
		AddFace(Point000, Point010, Point110, Point100, FVector(0.f, 0.f, -1.f));
	}
}

void AVoxelChunkActor::AddFace(const FVector& V0, const FVector& V1, const FVector& V2, const FVector& V3, const FVector& Normal)
{
	const int32 StartIndex = Vertices.Num();

	//한 면 mesh 만드는
	Vertices.Add(V0);
	Vertices.Add(V1);
	Vertices.Add(V2);
	Vertices.Add(V3);

	Triangles.Add(StartIndex + 0);
	Triangles.Add(StartIndex + 2);
	Triangles.Add(StartIndex + 1);

	Triangles.Add(StartIndex + 0);
	Triangles.Add(StartIndex + 3);
	Triangles.Add(StartIndex + 2);

	Normals.Add(Normal);
	Normals.Add(Normal);
	Normals.Add(Normal);
	Normals.Add(Normal);


	UVs.Add(FVector2D(0.f, 0.f));
	UVs.Add(FVector2D(1.f, 0.f));
	UVs.Add(FVector2D(1.f, 1.f));
	UVs.Add(FVector2D(0.f, 1.f));

	VertexColors.Add(FLinearColor::White);
	VertexColors.Add(FLinearColor::White);
	VertexColors.Add(FLinearColor::White);
	VertexColors.Add(FLinearColor::White);

	const FVector TangentX = (V1 - V0).GetSafeNormal();

	Tangents.Add(FProcMeshTangent(TangentX, false));
	Tangents.Add(FProcMeshTangent(TangentX, false));
	Tangents.Add(FProcMeshTangent(TangentX, false));
	Tangents.Add(FProcMeshTangent(TangentX, false));

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
					Voxels[GetVoxelIndex(X, Y, Z)] = false;
					
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

