// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/OverlapResult.h"
#include "VoxelWorld.generated.h"

class AVoxelChunkActor;
class ABaseProjectile;
enum class EVoxelBlockType : uint8;
struct FOverlapResult;

UCLASS()
class ONETWOSHOOT_API AVoxelWorld : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVoxelWorld();

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void RefreshChunkList();

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void RegisterChunk(AVoxelChunkActor* Chunk);

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void UnregisterChunk(AVoxelChunkActor* Chunk);

	UFUNCTION(BlueprintCallable, Category = "Voxel|World|Debug")
	void DebugDestroyAllChunks();

	UFUNCTION(BlueprintCallable, Category = "Voxel|World|Debug")
	void DebugDestroyRandomChunk();

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void DestroyVoxelsAtWorldLocation(FVector WorldLocation, float Radius);

	void RequestVoxelExplosionFromChunk(
		AVoxelChunkActor* ReportingChunk,
		FVector ExplosionOrigin,
		float ExplosionRadius,
		AActor* DamageCauser
	);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|World|Debug")
	float DebugDestroyRadius = 200.f;

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void HandleProjectileExplosion(FVector HitLocation, float ExplosionRadius);

	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void BindExistingProjectiles();


	// 아래는 새로 추가된 함수 or 변수들
	// 특정 복쉘 좌표에 다른 캐릭터(Pawn)가 서 있는지 확인
	bool IsVoxelOccupied(FIntVector Coords, AActor* IgnoreActor = nullptr);

	// 지형과 액터의 유무 등을 고려하여 이동 가능 여부를 판별
	bool IsWalkable(FIntVector Coords, AActor* IgnoreActor = nullptr, FIntVector TargetCoords = FIntVector(-99999));

	//전역 좌표를 기준으로 해당 위치의 복셀 타입을 가져옴
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	EVoxelBlockType GetVoxelTypeAt(FIntVector GlobalCoords);

	//월드 좌표 -> 복셀 좌표
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	FIntVector WorldToVoxelCoords(FVector WorldLocation) const;

	//복셀 좌표 -> 월드 좌표
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	FVector VoxelToWorldLocation(FIntVector VoxelCoords) const;

	// 캐릭터가 허공에 있거나 땅속에 있을 때, 가장 가까운 지표면을 찾음
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	FIntVector GetNearestWalkableVoxel(FVector WorldLocation);

	// 복쉘 한 변의 길이를 정의, VoxelChunkActor와 연동되지 않았으므로 주의
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|World")
	float VoxelSize = 100.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel|World")
	TArray<TObjectPtr<AVoxelChunkActor>> Chunks;

	UPROPERTY()
	TWeakObjectPtr<AActor> LastExplosionDamageCauser;

	float LastExplosionTime = -1.0f;
	FVector LastExplosionOrigin = FVector::ZeroVector;
	float LastExplosionRadius = 0.0f;
};
