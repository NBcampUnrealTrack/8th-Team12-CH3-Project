#include "../Public/World/VoxelChunkActor.h"
#include "../Public/World/VoxelBlockyMesher.h"
#include "../Public/World/VoxelMarchingMesher.h"
#include "../Public/World/VoxelWorld.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"


AVoxelChunkActor::AVoxelChunkActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = Mesh;
	Mesh->bUseAsyncCooking = true;

	SetCanBeDamaged(true);

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCanEverAffectNavigation(true);
	Mesh->SetCastShadow(false);


	VoxelMaterials.SetNum(GetVoxelTerrainMaterialSectionCount());
	DefaultBlockType = EVoxelBlockType::Dirt;
}

void AVoxelChunkActor::SetOwningVoxelWorld(AVoxelWorld* InVoxelWorld)
{
	OwningVoxelWorld = InVoxelWorld;
}

void AVoxelChunkActor::SetVoxelSize(float NewVoxelSize)
{
	VoxelSize = FMath::Max(1.0f, NewVoxelSize);
}

void AVoxelChunkActor::SetChunkDimensions(int32 NewSizeX, int32 NewSizeY, int32 NewSizeZ)
{
	ChunkSizeX = FMath::Max(1, NewSizeX);
	ChunkSizeY = FMath::Max(1, NewSizeY);
	ChunkSizeZ = FMath::Max(1, NewSizeZ);

	ChunkSize = ChunkSizeX; // 기존 코드 호환용. 나중엔 제거 후보.
	Voxels.Empty();
	MarchingVoxels.Empty();
}

void AVoxelChunkActor::BeginPlay()
{
	UpdateChunkVoxelOffsetFromLocation();

	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("VoxelChunk Initialized: %d voxels"), Voxels.Num());
	
	RebuildChunk();
}


//테스트용 잠시 주석처리
//void AVoxelChunkActor::InitializeVoxels()
//{
//	const int32 TotalVoxelCount = ChunkSizeX * ChunkSizeY * ChunkSizeZ;
//
//	Voxels.SetNum(TotalVoxelCount);
//
//	for (int32 i = 0; i < Voxels.Num(); i++)
//	{
//		Voxels[i] = MakeVoxelData(EVoxelBlockType::Dirt);
//	}
//}

void AVoxelChunkActor::InitializeVoxels()
{
	const int32 TotalVoxelCount = ChunkSizeX * ChunkSizeY * ChunkSizeZ;
	Voxels.SetNum(TotalVoxelCount);

	for (FVoxelData& Voxel : Voxels)
	{
		Voxel = MakeVoxelData(DefaultBlockType);
	}
}

void AVoxelChunkActor::InitializeMarchingVoxels()
{
	const int32 SampleCountX = ChunkSizeX + 1;
	const int32 SampleCountY = ChunkSizeY + 1;
	const int32 SampleCountZ = ChunkSizeZ + 1;
	const int32 TotalSampleCount = SampleCountX * SampleCountY * SampleCountZ;
	MarchingVoxels.SetNum(TotalSampleCount);

	for (FVoxelData& Voxel : MarchingVoxels)
	{
		Voxel = MakeVoxelData(DefaultBlockType);
	}
}

void AVoxelChunkActor::EnsureVoxelDataInitialized()
{
	if (!HasValidVoxelData())
	{
		InitializeVoxels();
	}

	if (!HasValidMarchingVoxelData())
	{
		InitializeMarchingVoxels();
	}
}

bool AVoxelChunkActor::HasValidVoxelData() const
{
	const int32 ExpectedVoxelCount = ChunkSizeX * ChunkSizeY * ChunkSizeZ;
	return ExpectedVoxelCount > 0 && Voxels.Num() == ExpectedVoxelCount;
}

bool AVoxelChunkActor::HasValidMarchingVoxelData() const
{
	const int32 ExpectedVoxelCount = (ChunkSizeX + 1) * (ChunkSizeY + 1) * (ChunkSizeZ + 1);
	return ExpectedVoxelCount > 0 && MarchingVoxels.Num() == ExpectedVoxelCount;
}

void AVoxelChunkActor::UpdateChunkVoxelOffsetFromLocation()
{
	// 자신의 월드 위치를 VoxelSize로 나누어 실제 복셀 좌표(Offset)를 계산합니다.
	ChunkVoxelOffset.X = FMath::FloorToInt(GetActorLocation().X / VoxelSize);
	ChunkVoxelOffset.Y = FMath::FloorToInt(GetActorLocation().Y / VoxelSize);
	ChunkVoxelOffset.Z = FMath::FloorToInt(GetActorLocation().Z / VoxelSize);

	UE_LOG(LogTemp, Warning, TEXT("[%s] 위치 기반 오프셋 설정 완료: %s"),
		*GetName(), *ChunkVoxelOffset.ToString());
}

int32 AVoxelChunkActor::GetVoxelIndex(int32 X, int32 Y, int32 Z) const
{
	const int32 Index = X + (Y * ChunkSizeX) + (Z * ChunkSizeX * ChunkSizeY);
	//UE_LOG(LogTemp,Warning,TEXT("VoxelIndex: X=%d Y=%d Z=%d | Dec=%d Hex=0x%X"),X, Y, Z, Index, Index);
	return Index;	
}

int32 AVoxelChunkActor::GetMarchingVoxelIndex(int32 X, int32 Y, int32 Z) const
{
	const int32 SampleCountX = ChunkSizeX + 1;
	const int32 SampleCountY = ChunkSizeY + 1;
	return X + (Y * SampleCountX) + (Z * SampleCountX * SampleCountY);
}

bool AVoxelChunkActor::IsCoordinateValid(int32 X, int32 Y, int32 Z) const
{
	return X >= 0 && X < ChunkSizeX &&
		Y >= 0 && Y < ChunkSizeY &&
		Z >= 0 && Z < ChunkSizeZ;
}

bool AVoxelChunkActor::IsMarchingCoordinateValid(int32 X, int32 Y, int32 Z) const
{
	return X >= 0 && X <= ChunkSizeX &&
		Y >= 0 && Y <= ChunkSizeY &&
		Z >= 0 && Z <= ChunkSizeZ;
}

bool AVoxelChunkActor::IsVoxelSolid(int32 X, int32 Y, int32 Z) const
{
	if (!IsCoordinateValid(X, Y, Z))
	{
		return false;
	}

	return Voxels[GetVoxelIndex(X, Y, Z)].BlockType != EVoxelBlockType::Air;
}

void AVoxelChunkActor::GenerateMesh()
{
	EnsureVoxelDataInitialized();

	switch (RenderMode)
	{
	case EVoxelRenderMode::Blocky:
		GenerateBlockyMesh();
		break;
	case EVoxelRenderMode::Marching:
		GenerateMarchingMesh();
		break;
	default:
		GenerateBlockyMesh();
		break;
	}
}

void AVoxelChunkActor::GenerateBlockyMesh()
{
	FVoxelBlockyMesher::Generate(
		FVoxelBlockyMesherSettings{
			ChunkSizeX,
			ChunkSizeY,
			ChunkSizeZ,
			VoxelSize,
			[this](int32 X, int32 Y, int32 Z)
			{
				if (!IsCoordinateValid(X, Y, Z))
				{
					return FVoxelData();
				}

				const int32 VoxelIndex = GetVoxelIndex(X, Y, Z);
				return Voxels.IsValidIndex(VoxelIndex) ? Voxels[VoxelIndex] : FVoxelData();
			}
		},
		MeshSections
	);

	CreateMeshSections();

	int32 TotalVertices = 0;
	int32 TotalTriangles = 0;
	for (const FChunkMeshData& MeshData : MeshSections)
	{
		TotalVertices += MeshData.Vertices.Num();
		TotalTriangles += MeshData.Triangles.Num() / 3;
	}

	UE_LOG(LogTemp, Warning, TEXT("Generated Mesh: Sections=%d / Vertices=%d / Triangles=%d"), MeshSections.Num(), TotalVertices, TotalTriangles);
}

void AVoxelChunkActor::GenerateMarchingMesh()
{
	FVoxelMarchingMesher::Generate(
		FVoxelMarchingMesherSettings{
			ChunkSizeX,
			ChunkSizeY,
			ChunkSizeZ,
			VoxelSize,
			[this](int32 X, int32 Y, int32 Z)
			{
				return GetMarchingSample(X, Y, Z);
			}
		},
		MeshSections
	);

	CreateMeshSections();

	int32 TotalVertices = 0;
	int32 TotalTriangles = 0;
	for (const FChunkMeshData& MeshData : MeshSections)
	{
		TotalVertices += MeshData.Vertices.Num();
		TotalTriangles += MeshData.Triangles.Num() / 3;
	}

	UE_LOG(LogTemp, Warning, TEXT("Generated Marching Mesh: Sections=%d / Vertices=%d / Triangles=%d"), MeshSections.Num(), TotalVertices, TotalTriangles);
}

void AVoxelChunkActor::CreateMeshSections()
{
	Mesh->ClearAllMeshSections();

	for (int32 SectionIndex = 0; SectionIndex < MeshSections.Num(); SectionIndex++)
	{
		const FChunkMeshData& MeshData = MeshSections[SectionIndex];
		if (MeshData.Vertices.Num() > 0)
		{
			Mesh->CreateMeshSection_LinearColor(
				SectionIndex,
				MeshData.Vertices,
				MeshData.Triangles,
				MeshData.Normals,
				MeshData.UVs,
				MeshData.VertexColors,
				MeshData.Tangents,
				true
			);
		}

		UMaterialInterface* SectionMaterial = VoxelMaterials.IsValidIndex(SectionIndex)
			? VoxelMaterials[SectionIndex]
			: nullptr;
		if (!SectionMaterial)
		{
			SectionMaterial = FallbackVoxelMaterial;
		}
		if (SectionMaterial)
		{
			Mesh->SetMaterial(SectionIndex, SectionMaterial);
		}
	}
}

void AVoxelChunkActor::RebuildChunk()
{
	if (!Mesh) return;

	UpdateChunkVoxelOffsetFromLocation();
	if (!OwningVoxelWorld.IsValid())
	{
		OwningVoxelWorld = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass()));
	}
	EnsureVoxelDataInitialized();
	GenerateMesh();
}

void AVoxelChunkActor::SetRenderSettings(EVoxelRenderMode NewRenderMode)
{
	RenderMode = NewRenderMode;
}

FVoxelData AVoxelChunkActor::GetMarchingSample(int32 X, int32 Y, int32 Z) const
{
	if (const AVoxelWorld* VoxelWorld = OwningVoxelWorld.Get())
	{
		return VoxelWorld->GetVoxelDataAt(ChunkVoxelOffset + FIntVector(X, Y, Z));
	}

	if (!IsMarchingCoordinateValid(X, Y, Z))
	{
		return FVoxelData();
	}

	const int32 VoxelIndex = GetMarchingVoxelIndex(X, Y, Z);
	return MarchingVoxels.IsValidIndex(VoxelIndex) ? MarchingVoxels[VoxelIndex] : FVoxelData();
}

void AVoxelChunkActor::SetMarchingSample(FIntVector LocalCoords, const FVoxelData& VoxelData)
{
	if (!IsMarchingCoordinateValid(LocalCoords.X, LocalCoords.Y, LocalCoords.Z))
	{
		return;
	}

	const int32 VoxelIndex = GetMarchingVoxelIndex(LocalCoords.X, LocalCoords.Y, LocalCoords.Z);
	if (MarchingVoxels.IsValidIndex(VoxelIndex))
	{
		MarchingVoxels[VoxelIndex] = VoxelData;
	}
}

bool AVoxelChunkActor::SetVoxel(FIntVector LocalCoords, EVoxelBlockType NewType, bool bRebuildMesh)
{
	EnsureVoxelDataInitialized();

	if (!IsCoordinateValid(LocalCoords.X, LocalCoords.Y, LocalCoords.Z))
	{
		return false;
	}

	const int32 VoxelIndex = GetVoxelIndex(LocalCoords.X, LocalCoords.Y, LocalCoords.Z);
	if (!Voxels.IsValidIndex(VoxelIndex))
	{
		return false;
	}

	Voxels[VoxelIndex] = MakeVoxelData(NewType);
	const FVoxelData NewVoxelData = MakeVoxelData(NewType);
	for (int32 Z = 0; Z <= 1; Z++)
	{
		for (int32 Y = 0; Y <= 1; Y++)
		{
			for (int32 X = 0; X <= 1; X++)
			{
				SetMarchingSample(LocalCoords + FIntVector(X, Y, Z), NewVoxelData);
			}
		}
	}

	if (bRebuildMesh)
	{
		GenerateMesh();
	}

	return true;
}

bool AVoxelChunkActor::RemoveVoxel(FIntVector LocalCoords, bool bRebuildMesh)
{
	return SetVoxel(LocalCoords, EVoxelBlockType::Air, bRebuildMesh);
}

EVoxelBlockType AVoxelChunkActor::GetLocalVoxelType(FIntVector LocalCoords) const
{
	if (!IsCoordinateValid(LocalCoords.X, LocalCoords.Y, LocalCoords.Z))
	{
		return EVoxelBlockType::Air;
	}

	const int32 VoxelIndex = GetVoxelIndex(LocalCoords.X, LocalCoords.Y, LocalCoords.Z);
	return Voxels.IsValidIndex(VoxelIndex) ? Voxels[VoxelIndex].BlockType : EVoxelBlockType::Air;
}

FVoxelData AVoxelChunkActor::GetLocalVoxelData(FIntVector LocalCoords) const
{
	if (IsMarchingCoordinateValid(LocalCoords.X, LocalCoords.Y, LocalCoords.Z))
	{
		const int32 MarchingVoxelIndex = GetMarchingVoxelIndex(LocalCoords.X, LocalCoords.Y, LocalCoords.Z);
		if (MarchingVoxels.IsValidIndex(MarchingVoxelIndex))
		{
			return MarchingVoxels[MarchingVoxelIndex];
		}
	}

	if (!IsCoordinateValid(LocalCoords.X, LocalCoords.Y, LocalCoords.Z))
	{
		return FVoxelData();
	}

	const int32 VoxelIndex = GetVoxelIndex(LocalCoords.X, LocalCoords.Y, LocalCoords.Z);
	return Voxels.IsValidIndex(VoxelIndex) ? Voxels[VoxelIndex] : FVoxelData();
}

FVoxelData AVoxelChunkActor::MakeVoxelData(EVoxelBlockType BlockType) const
{
	FVoxelData VoxelData;
	VoxelData.BlockType = BlockType;
	VoxelData.Density = BlockType == EVoxelBlockType::Air ? 1.0f : -1.0f;
	return VoxelData;
}

FVector AVoxelChunkActor::GetRandomWorldLocationInsideChunk() const
{
	const FVector RandomLocalLocation = FVector(
		FMath::FRandRange(0.f, ChunkSizeX * VoxelSize),
		FMath::FRandRange(0.f, ChunkSizeY * VoxelSize),
		FMath::FRandRange(0.f, ChunkSizeZ * VoxelSize)
	);

	return GetActorTransform().TransformPosition(RandomLocalLocation);
}
bool AVoxelChunkActor::DestroyVoxelsAtWorldLocation(FVector WorldLocation, float Radius, bool bRebuildMesh)
{
	EnsureVoxelDataInitialized();

	const FVector LocalLocation = GetActorTransform().InverseTransformPosition(WorldLocation);

	const int32 CenterX = FMath::FloorToInt(LocalLocation.X / VoxelSize);
	const int32 CenterY = FMath::FloorToInt(LocalLocation.Y / VoxelSize);
	const int32 CenterZ = FMath::FloorToInt(LocalLocation.Z / VoxelSize);

	const int32 RadiusInVoxels = FMath::CeilToInt(Radius / VoxelSize);
	bool bAnyVoxelDestroyed = false;

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
					const int32 VoxelIndex = GetVoxelIndex(X, Y, Z);
					if (Voxels[VoxelIndex].BlockType != EVoxelBlockType::Air)
					{
						Voxels[VoxelIndex] = MakeVoxelData(EVoxelBlockType::Air);
						bAnyVoxelDestroyed = true;
					}
				}
			}
		}
	}

	for (int32 Z = CenterZ - RadiusInVoxels; Z <= CenterZ + RadiusInVoxels + 1; Z++)
	{
		for (int32 Y = CenterY - RadiusInVoxels; Y <= CenterY + RadiusInVoxels + 1; Y++)
		{
			for (int32 X = CenterX - RadiusInVoxels; X <= CenterX + RadiusInVoxels + 1; X++)
			{
				if (!IsMarchingCoordinateValid(X, Y, Z)) continue;

				const FVector SampleLocation = FVector(X * VoxelSize, Y * VoxelSize, Z * VoxelSize);
				if (FVector::Dist(SampleLocation, LocalLocation) <= Radius)
				{
					const int32 MarchingVoxelIndex = GetMarchingVoxelIndex(X, Y, Z);
					if (MarchingVoxels.IsValidIndex(MarchingVoxelIndex) && MarchingVoxels[MarchingVoxelIndex].BlockType != EVoxelBlockType::Air)
					{
						MarchingVoxels[MarchingVoxelIndex] = MakeVoxelData(EVoxelBlockType::Air);
						bAnyVoxelDestroyed = true;
					}
				}
			}
		}
	}
	//TRACE_CPUPROFILER_EVENT_SCOPE(Voxel_GenerateMesh);

	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Generate Mesh Called."));
	if (bAnyVoxelDestroyed && bRebuildMesh)
	{
		GenerateMesh();
	}

	//BP_OnDebugDestroyed(WorldLocation, Radius);
	return bAnyVoxelDestroyed;
}

float AVoxelChunkActor::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser
	);

	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		const FRadialDamageEvent& RadialDamageEvent =
			static_cast<const FRadialDamageEvent&>(DamageEvent);

		const FVector ExplosionOrigin = RadialDamageEvent.Origin;
		const float ExplosionRadius = RadialDamageEvent.Params.OuterRadius;

		if (!OwningVoxelWorld.IsValid())
		{
			OwningVoxelWorld = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass()));
		}

		if (AVoxelWorld* VoxelWorld = OwningVoxelWorld.Get())
		{
			VoxelWorld->RequestVoxelExplosionFromChunk(this, ExplosionOrigin, ExplosionRadius, DamageCauser);
		}

		UE_LOG(LogTemp,Warning,TEXT("VoxelChunk reported radial damage. Damage=%.1f Origin=%s Radius=%.1f"),ActualDamage,*ExplosionOrigin.ToString(),ExplosionRadius);
	}
	return ActualDamage;
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

// 전역 좌표가 청크의 시작점(Offset)과 끝점(Offset + Size) 사이에 있는지 확인
bool AVoxelChunkActor::Contains(FIntVector GlobalCoords) const
{
	return (GlobalCoords.X >= ChunkVoxelOffset.X && GlobalCoords.X < ChunkVoxelOffset.X + ChunkSizeX) &&
		(GlobalCoords.Y >= ChunkVoxelOffset.Y && GlobalCoords.Y < ChunkVoxelOffset.Y + ChunkSizeY) &&
		(GlobalCoords.Z >= ChunkVoxelOffset.Z && GlobalCoords.Z < ChunkVoxelOffset.Z + ChunkSizeZ);
}

// 현재 복셀 청크가 어떤 타입의 블럭인지 확인
EVoxelBlockType AVoxelChunkActor::GetVoxelType(FIntVector GlobalCoords) const
{
	int32 LocalX = GlobalCoords.X - ChunkVoxelOffset.X;
	int32 LocalY = GlobalCoords.Y - ChunkVoxelOffset.Y;
	int32 LocalZ = GlobalCoords.Z - ChunkVoxelOffset.Z;
	return GetLocalVoxelType(FIntVector(LocalX, LocalY, LocalZ));
}

FBox AVoxelChunkActor::GetChunkWorldBounds(float Padding) const
{
	const FVector Min = GetActorLocation();
	const FVector Max = Min + FVector(
		ChunkSizeX * VoxelSize,
		ChunkSizeY * VoxelSize,
		ChunkSizeZ * VoxelSize
	);
	return FBox(Min, Max).ExpandBy(Padding);
}

