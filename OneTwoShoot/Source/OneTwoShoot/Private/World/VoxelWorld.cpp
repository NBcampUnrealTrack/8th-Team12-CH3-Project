// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/World/VoxelWorld.h"
#include "../Public/Tank/BaseProjectile.h"
#include "../Public/World/VoxelChunkActor.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
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

bool AVoxelWorld::IsVoxelOccupied(FIntVector Coords, AActor* IgnoreActor)
{
	FVector WorldPos = VoxelToWorldLocation(Coords);
	WorldPos.Z += 50.0f;

	FCollisionQueryParams Params;
	if (IgnoreActor) Params.AddIgnoredActor(IgnoreActor);

	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(40.0f);

	return GetWorld()->OverlapMultiByChannel(Overlaps, WorldPos, FQuat::Identity, ECC_Pawn, Sphere, Params);
}

bool AVoxelWorld::IsWalkable(FIntVector Coords, AActor* IgnoreActor, FIntVector TargetCoords)
{
	if (GetVoxelTypeAt(Coords + FIntVector(0, 0, -1)) == EVoxelBlockType::Air) return false;
	if (GetVoxelTypeAt(Coords) != EVoxelBlockType::Air) return false;

	FIntVector MyCurrentCoords = WorldToVoxelCoords(IgnoreActor ? IgnoreActor->GetActorLocation() : FVector::ZeroVector);

	if (Coords == TargetCoords || Coords == MyCurrentCoords) return true;

	return !IsVoxelOccupied(Coords, IgnoreActor);
}

// 전역 좌표를 통해 특정 청크의 복쉘 타입을 가져옴
EVoxelBlockType AVoxelWorld::GetVoxelTypeAt(FIntVector GlobalCoords)
{
	for (AVoxelChunkActor* Chunk : Chunks)
	{
		if (Chunk->Contains(GlobalCoords))
		{
			return Chunk->GetVoxelType(GlobalCoords);
		}
	}

	UE_LOG(LogTemp, Error, TEXT("좌표 %s 에 해당하는 청크를 찾을 수 없음!"), *GlobalCoords.ToString());
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

FIntVector AVoxelWorld::GetNearestWalkableVoxel(FVector WorldLocation)
{
	FIntVector Coords = WorldToVoxelCoords(WorldLocation);

	if (GetVoxelTypeAt(Coords) != EVoxelBlockType::Air)
	{
		while (GetVoxelTypeAt(Coords) != EVoxelBlockType::Air && Coords.Z < 100)
		{
			Coords.Z++;
		}
	}
	else
	{
		while (GetVoxelTypeAt(Coords + FIntVector(0, 0, -1)) == EVoxelBlockType::Air && Coords.Z > -100)
		{
			Coords.Z--;
		}
	}

	return Coords;
}