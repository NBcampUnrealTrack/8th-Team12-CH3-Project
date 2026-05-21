// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/World/VoxelWorld.h"
#include "../Public/World/VoxelChunkActor.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h" 
#include "GameFramework/Actor.h"

// 월드 매니저는 매 프레임 처리할 일이 없으므로 Tick을 꺼둔다.
AVoxelWorld::AVoxelWorld()
{
	PrimaryActorTick.bCanEverTick = false;
}


// BeginPlay 시점에 레벨에 이미 배치된 청크들을 한 번 수집한다.
void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	RefreshChunkList();
}

// 레벨 안의 모든 AVoxelChunkActor를 찾아 Chunks 배열을 새로 만든다.
// RegisterChunk를 통해 청크 쪽 OwningVoxelWorld도 같이 설정한다.
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

// 관리 대상 청크를 추가하고, 청크가 월드 조회를 할 수 있도록 역참조를 연결한다.
// AddUnique를 사용해 같은 청크가 중복 등록되지 않게 한다.
void AVoxelWorld::RegisterChunk(AVoxelChunkActor* Chunk)
{
	if (!IsValid(Chunk))
	{
		return;
	}

	Chunks.AddUnique(Chunk);
	Chunk->SetOwningVoxelWorld(this);
}

// 청크가 제거될 때 월드 목록과 청크 내부 역참조를 함께 정리한다.
void AVoxelWorld::UnregisterChunk(AVoxelChunkActor* Chunk)
{
	if (!Chunk)
	{
		return;
	}

	Chunks.Remove(Chunk);
	Chunk->SetOwningVoxelWorld(nullptr);
}

// 여러 청크가 같은 RadialDamage를 각각 보고할 수 있으므로,
// 폭발 위치/반경/시간이 거의 같은 요청은 하나만 처리한다.
void AVoxelWorld::RequestVoxelExplosionFromChunk(
	AVoxelChunkActor* ReportingChunk,
	FVector ExplosionOrigin,
	float ExplosionRadius,
	AActor* DamageCauser
)
{
	const UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;

	// 같은 폭발 데미지가 여러 청크에 들어온 경우 0.5초 안의 유사 요청을 중복으로 본다.
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

	// 이번 폭발을 마지막 처리 기록으로 저장한 뒤 실제 지형 파괴를 실행한다.
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

// 이름은 전체 파괴처럼 보이지만 현재 구현은 랜덤 청크 파괴 테스트로 연결되어 있다.
void AVoxelWorld::DebugDestroyAllChunks()
{
	DebugDestroyRandomChunk();
}

// 유효하지 않은 청크 참조를 정리한 뒤, 임의 청크 내부의 임의 위치에 디버그 폭발을 만든다.
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

// 폭발 반경과 겹치는 청크의 데이터부터 모두 수정한 뒤, 경계 샘플을 공유할 수 있는 이웃 청크까지 함께 재빌드한다.
// Marching Cubes는 이웃 청크의 샘플을 읽으므로, 데이터 수정 중간에 청크별로 즉시 GenerateMesh를 호출하면 경계 잔상이 남을 수 있다.
void AVoxelWorld::DestroyVoxelsAtWorldLocation(FVector WorldLocation, float Radius)
{
	TSet<AVoxelChunkActor*> ChunksToRebuild;
	bool bAnyVoxelDestroyed = false;
	const float RebuildPadding = FMath::Max(VoxelSize, 1.0f);

	for (AVoxelChunkActor* Chunk : Chunks)
	{
		if (!IsValid(Chunk))
		{
			continue;
		}

		const FBox ChunkBounds = Chunk->GetChunkWorldBounds();
		const float DistanceToChunkSq = ChunkBounds.ComputeSquaredDistanceToPoint(WorldLocation);
		if (DistanceToChunkSq <= FMath::Square(Radius))
		{
			if (Chunk->DestroyVoxelsAtWorldLocation(WorldLocation, Radius, false))
			{
				bAnyVoxelDestroyed = true;
				ChunksToRebuild.Add(Chunk);
			}
		}
	}

	if (!bAnyVoxelDestroyed)
	{
		return;
	}

	for (AVoxelChunkActor* Chunk : Chunks)
	{
		if (!IsValid(Chunk))
		{
			continue;
		}

		const FBox ChunkBounds = Chunk->GetChunkWorldBounds(RebuildPadding);
		const float DistanceToChunkSq = ChunkBounds.ComputeSquaredDistanceToPoint(WorldLocation);
		if (DistanceToChunkSq <= FMath::Square(Radius + RebuildPadding))
		{
			ChunksToRebuild.Add(Chunk);
		}
	}

	for (AVoxelChunkActor* Chunk : ChunksToRebuild)
	{
		if (IsValid(Chunk))
		{
			Chunk->RebuildChunk();
		}
	}
}

// 에디터에서 렌더 설정을 바꾼 뒤 모든 청크에 같은 설정을 적용하고 다시 빌드한다.
// RefreshChunkList를 먼저 호출해 현재 레벨 상태를 기준으로 처리한다.
void AVoxelWorld::RebuildAllChunks()
{
	RefreshChunkList();

	for (AVoxelChunkActor* Chunk : Chunks)
	{
		if (!IsValid(Chunk))
		{
			continue;
		}

		Chunk->SetRenderSettings(RenderMode);
		Chunk->RebuildChunk();
	}
}

// 복셀 좌표 중심 근처에 Pawn 오버랩이 있는지 검사한다.
// IgnoreActor는 자기 자신을 경로 탐색 장애물로 보지 않게 하기 위한 예외 대상이다.
bool AVoxelWorld::IsVoxelOccupied(FIntVector Coords, AActor* IgnoreActor)
{
	FVector WorldPos = VoxelToWorldLocation(Coords);
	WorldPos.Z += 50.0f;

	FCollisionQueryParams Params;
	if (IgnoreActor) Params.AddIgnoredActor(IgnoreActor);

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerChar)
	{
		Params.AddIgnoredActor(PlayerChar);
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(40.0f);

	return GetWorld()->OverlapMultiByChannel(Overlaps, WorldPos, FQuat::Identity, ECC_Pawn, Sphere, Params);
}

// 이동 가능한 칸인지 검사한다.
// 아래 칸은 solid여야 하고, 현재 칸은 Air여야 하며, 목적지/현재 위치 예외가 아니면 Pawn 점유도 없어야 한다.
bool AVoxelWorld::IsWalkable(FIntVector Coords, AActor* IgnoreActor, FIntVector TargetCoords)
{
	if (GetVoxelTypeAt(Coords + FIntVector(0, 0, -1)) == EVoxelBlockType::Air) return false;
	if (GetVoxelTypeAt(Coords) != EVoxelBlockType::Air) return false;

	FIntVector MyCurrentCoords = WorldToVoxelCoords(IgnoreActor ? IgnoreActor->GetActorLocation() : FVector::ZeroVector);

	if (Coords == TargetCoords || Coords == MyCurrentCoords) return true;

	return !IsVoxelOccupied(Coords, IgnoreActor);
}

// 전역 복셀 좌표를 포함하는 청크를 찾고, 그 청크의 로컬 데이터에서 블록 타입만 반환한다.
// 어떤 청크도 포함하지 않으면 Air로 취급한다.
EVoxelBlockType AVoxelWorld::GetVoxelTypeAt(FIntVector GlobalCoords)
{
	for (AVoxelChunkActor* Chunk : Chunks)
	{
		if (Chunk->Contains(GlobalCoords))
		{
			return Chunk->GetVoxelType(GlobalCoords);
		}
	}

	//UE_LOG(LogTemp, Error, TEXT("좌표 %s 에 해당하는 청크를 찾을 수 없음!"), *GlobalCoords.ToString());
	return EVoxelBlockType::Air;
}

// Marching Cubes용 전역 샘플 조회 함수.
// 청크 경계에서 현재 청크가 이웃 청크의 샘플을 읽을 수 있도록 전역 좌표 기반으로 검색한다.
FVoxelData AVoxelWorld::GetVoxelDataAt(FIntVector GlobalCoords) const
{
	for (AVoxelChunkActor* Chunk : Chunks)
	{
		if (IsValid(Chunk) && Chunk->Contains(GlobalCoords))
		{
			const FIntVector LocalCoords = GlobalCoords - Chunk->ChunkVoxelOffset;
			return Chunk->GetLocalVoxelData(LocalCoords);
		}
	}

	return FVoxelData();
}

// 월드 좌표가 어느 복셀 칸에 속하는지 계산한다.
// 음수 좌표도 올바르게 처리하기 위해 FloorToInt를 사용한다.
FIntVector AVoxelWorld::WorldToVoxelCoords(FVector WorldLocation) const
{
	return FIntVector(
		FMath::FloorToInt(WorldLocation.X / VoxelSize),
		FMath::FloorToInt(WorldLocation.Y / VoxelSize),
		FMath::FloorToInt(WorldLocation.Z / VoxelSize)
	);
}

// 복셀 좌표를 해당 칸의 월드 중심 위치로 변환한다.
FVector AVoxelWorld::VoxelToWorldLocation(FIntVector VoxelCoords) const
{
	return FVector(
		(VoxelCoords.X + 0.5f) * VoxelSize,
		(VoxelCoords.Y + 0.5f) * VoxelSize,
		(VoxelCoords.Z + 0.5f) * VoxelSize
	);
}

// 입력 위치가 지형 내부라면 위로 올라가며 빈 칸을 찾고,
// 허공이라면 아래로 내려가며 발밑 지형이 있는 위치를 찾는다.
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
