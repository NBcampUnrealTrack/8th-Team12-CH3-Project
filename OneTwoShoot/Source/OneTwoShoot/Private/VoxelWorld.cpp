// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelWorld.h"
#include "Kismet/GameplayStatics.h"
#include "VoxelChunkActor.h"
#include "BaseProjectile.h"


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

void AVoxelWorld::RegisterChunk(AVoxelChunkActor* Chunk) // 청크 등록
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

