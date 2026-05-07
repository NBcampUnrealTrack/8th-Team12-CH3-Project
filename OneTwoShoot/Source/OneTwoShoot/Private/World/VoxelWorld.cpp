// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/Tank/BaseProjectile.h"
#include "../Public/World/VoxelChunkActor.h"
#include "../Public/World/VoxelWorld.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AVoxelWorld::AVoxelWorld()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVoxelWorld::HandleProjectileExplosion(FVector HitLocation, float ExplosionRadius)
{
	DestroyVoxelsAtWorldLocation(HitLocation, ExplosionRadius);
}

void AVoxelWorld::BindExistingProjectiles()
{
	TArray<AActor*> FoundProjectiles;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		ABaseProjectile::StaticClass(),
		FoundProjectiles
	);

	for (AActor* Actor : FoundProjectiles)
	{
		ABaseProjectile* Projectile = Cast<ABaseProjectile>(Actor);
		if (!IsValid(Projectile))
		{
			continue;
		}

		Projectile->OnExplosionHit.AddDynamic(
			this,
			&AVoxelWorld::HandleProjectileExplosion
		);
	}
}

// Called when the game starts or when spawned
void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	RefreshChunkList();
}

void AVoxelWorld::RefreshChunkList()
{
	Chunks.Empty();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVoxelChunkActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		RegisterChunk(Cast<AVoxelChunkActor>(Actor));
	}

	UE_LOG(LogTemp, Warning, TEXT("VoxelWorld found %d voxel chunks."), Chunks.Num());
}

void AVoxelWorld::RegisterChunk(AVoxelChunkActor* Chunk) // ûũ ���
{
	if (!IsValid(Chunk))
	{
		return;
	}

	Chunks.AddUnique(Chunk);
	Chunk->SetOwningVoxelWorld(this);
}

void AVoxelWorld::UnregisterChunk(AVoxelChunkActor* Chunk)
{
	if (!Chunk)
	{
		return;
	}

	Chunks.Remove(Chunk);
	Chunk->SetOwningVoxelWorld(nullptr);
}

void AVoxelWorld::RequestVoxelExplosionFromChunk(
	AVoxelChunkActor* ReportingChunk,
	FVector ExplosionOrigin,
	float ExplosionRadius,
	AActor* DamageCauser
)
{
	const UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;

	const bool bWithinDuplicateWindow =	LastExplosionTime >= 0.0f && CurrentTime - LastExplosionTime <= 0.5f;
	const bool bSimilarOrigin =
		FVector::DistSquared(LastExplosionOrigin, ExplosionOrigin) <= FMath::Square(50.f);
	const bool bSimilarRadius =	FMath::Abs(LastExplosionRadius - ExplosionRadius) <= 50.f;

	if (bWithinDuplicateWindow && bSimilarOrigin && bSimilarRadius)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("VoxelWorld ignored duplicate explosion request. Origin=%s Radius=%.1f Reporter=%s Causer=%s"),
			*ExplosionOrigin.ToString(),
			ExplosionRadius,
			IsValid(ReportingChunk) ? *ReportingChunk->GetName() : TEXT("None"),
			IsValid(DamageCauser) ? *DamageCauser->GetName() : TEXT("None")
		);
		return;
	}

	LastExplosionTime = CurrentTime;
	LastExplosionOrigin = ExplosionOrigin;
	LastExplosionRadius = ExplosionRadius;
	LastExplosionDamageCauser = DamageCauser;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("VoxelWorld accepted explosion request. Origin=%s Radius=%.1f Reporter=%s Causer=%s"),
		*ExplosionOrigin.ToString(),
		ExplosionRadius,
		IsValid(ReportingChunk) ? *ReportingChunk->GetName() : TEXT("None"),
		IsValid(DamageCauser) ? *DamageCauser->GetName() : TEXT("None")
	);

	DestroyVoxelsAtWorldLocation(ExplosionOrigin, ExplosionRadius);
}

void AVoxelWorld::DebugDestroyAllChunks()
{
	DebugDestroyRandomChunk();
}

void AVoxelWorld::DebugDestroyRandomChunk()
{
	Chunks.RemoveAll([](const TObjectPtr<AVoxelChunkActor>& Chunk)
	{
		return !IsValid(Chunk);
	});

	if (Chunks.Num() == 0)
	{
		RefreshChunkList();
	}

	if (Chunks.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("VoxelWorld could not find any voxel chunks to debug destroy."));
		return;
	}

	const int32 RandomChunkIndex = FMath::RandRange(0, Chunks.Num() - 1);
	AVoxelChunkActor* SourceChunk = Chunks[RandomChunkIndex];
	if (!IsValid(SourceChunk))
	{
		return;
	}

	const FVector ExplosionLocation = SourceChunk->GetRandomWorldLocationInsideChunk();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("VoxelWorld debug explosion at %s / Radius: %.1f / SourceChunk: %s"),
		*ExplosionLocation.ToString(),
		DebugDestroyRadius,
		*SourceChunk->GetName()
	);

	DestroyVoxelsAtWorldLocation(ExplosionLocation, DebugDestroyRadius);
}

void AVoxelWorld::DestroyVoxelsAtWorldLocation(FVector WorldLocation, float Radius)
{
	for (AVoxelChunkActor* Chunk : Chunks)
	{
		if (!IsValid(Chunk))
		{
			continue;
		}

		const FBox ChunkBounds = Chunk->GetComponentsBoundingBox();
		const float DistanceToChunkSq = ChunkBounds.ComputeSquaredDistanceToPoint(WorldLocation);
		if (DistanceToChunkSq <= FMath::Square(Radius))
		{
			Chunk->DestroyVoxelsAtWorldLocation(WorldLocation, Radius);
		}
	}

}

// 전역 복쉘 좌표가 이동 가능한지 체크 (바닥, 벽 사이, 천장 세 가지 체크)
bool AVoxelWorld::IsWalkable(FIntVector Coords)
{
	// 1. 각 위치의 블록 상태 확인
	EVoxelBlockType Ground = GetVoxelTypeAt(Coords + FIntVector(0, 0, -1)); // 발밑
	EVoxelBlockType Body = GetVoxelTypeAt(Coords);                       // 몸통
	EVoxelBlockType Head = GetVoxelTypeAt(Coords + FIntVector(0, 0, 1));  // 머리

	// 2. [검사 1] 발밑에 땅이 있는가?
	if (Ground == EVoxelBlockType::Air)
	{
		// 너무 자주 찍히면 시끄러우니 경로 탐색 시에만 확인하거나 필요할 때 켭니다.
		UE_LOG(LogTemp, Error, TEXT("이동 불가 [%s]: 발밑(Z-1)이 공기입니다!"), *Coords.ToString());
		return false;
	}

	// 3. [검사 2] 내 몸과 머리 위치가 비어있는가? (벽/천장 체크)
	if (Body != EVoxelBlockType::Air)
	{
		UE_LOG(LogTemp, Error, TEXT("이동 불가 [%s]: 몸통 위치에 블록이 있습니다!"), *Coords.ToString());
		return false;
	}

	if (Head != EVoxelBlockType::Air)
	{
		UE_LOG(LogTemp, Error, TEXT("이동 불가 [%s]: 머리 위치에 블록이 있습니다!"), *Coords.ToString());
		return false;
	}

	// 모든 조건 통과
	return true;
}

// 전역 좌표를 통해 특정 청크의 복쉘 타입을 가져옴
EVoxelBlockType AVoxelWorld::GetVoxelTypeAt(FIntVector GlobalCoords)
{
	// 1. 어느 청크에 속하는지 계산
	// 2. 해당 청크 액터의 Voxels 배열에서 데이터 추출
	// (이 로직은 구현한 Chunk 관리 방식에 맞춰 추가 구현 필요)

	// 탐색하려는 좌표가 어떤 청크에 속하는지 로그를 찍어봅니다.
	for (AVoxelChunkActor* Chunk : Chunks)
	{
		if (Chunk->Contains(GlobalCoords)) // 청크가 이 좌표를 포함하는지 확인하는 함수가 있다면
		{
			return Chunk->GetVoxelType(GlobalCoords);
		}
	}

	// 여기까지 오면 해당 좌표에 청크가 없다는 뜻입니다!
	// UE_LOG(LogTemp, Error, TEXT("좌표 %s 에 해당하는 청크를 찾을 수 없음!"), *GlobalCoords.ToString());
	return EVoxelBlockType::Air;
}

FIntVector AVoxelWorld::WorldToVoxelCoords(FVector WorldLocation) const
{
	return FIntVector(
		FMath::FloorToInt(WorldLocation.X / VoxelSize),
		FMath::FloorToInt(WorldLocation.Y / VoxelSize),
		FMath::FloorToInt(WorldLocation.Z / VoxelSize)
	);
}

FVector AVoxelWorld::VoxelToWorldLocation(FIntVector VoxelCoords) const
{
	return FVector(
		(VoxelCoords.X + 0.5f) * VoxelSize,
		(VoxelCoords.Y + 0.5f) * VoxelSize,
		(VoxelCoords.Z + 0.5f) * VoxelSize
	);
}