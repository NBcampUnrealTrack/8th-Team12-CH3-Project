// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "VoxelDefinitions.h"
#include "VoxelChunkActor.generated.h"


class UProceduralMeshComponent;
class UMaterialInterface;
class AVoxelWorld;

UCLASS()
class ONETWOSHOOT_API AVoxelChunkActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVoxelChunkActor();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Voxel|Actions")
	void RebuildChunk();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Render")
	void SetRenderSettings(EVoxelRenderMode NewRenderMode);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Debug")
	bool DestroyVoxelsAtWorldLocation(FVector WorldLocation, float Radius, bool bRebuildMesh = true);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Edit")
	bool SetVoxel(FIntVector LocalCoords, EVoxelBlockType NewType, bool bRebuildMesh = true);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Edit")
	bool RemoveVoxel(FIntVector LocalCoords, bool bRebuildMesh = true);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel|Edit")
	EVoxelBlockType GetLocalVoxelType(FIntVector LocalCoords) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel|Edit")
	FVoxelData GetLocalVoxelData(FIntVector LocalCoords) const;

	void SetOwningVoxelWorld(AVoxelWorld* InVoxelWorld);

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;



	UFUNCTION(BlueprintCallable, Category = "Voxel|Debug")
	void DebugDestroyCenter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug") // Obsolete
	float DebugDestroyRadius = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug") // Obsolete
	FVector DebugDestroyLocalOffset = FVector::ZeroVector;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel") // 테스트용 함수
	FVector GetRandomWorldLocationInsideChunk() const;

	// 아래는 새로 추가된 함수 or 변수들
	// 이 청크가 전역 복쉘 좌표를 포함하고 있는지 확인
	bool Contains(FIntVector GlobalCoords) const;

	// 전역 좌표를 받아 청크 내부의 복쉘 타입을 반환
	EVoxelBlockType GetVoxelType(FIntVector GlobalCoords) const;

	// 현재 메쉬 모양과 무관하게 청크가 차지하는 전체 월드 그리드 bounds를 반환한다.
	FBox GetChunkWorldBounds(float Padding = 0.0f) const;

	// 청크가 시작되는 전역 복쉘 좌표
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FIntVector ChunkVoxelOffset;

	// 청크의 한 변 길이
	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 ChunkSize = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Render")
	EVoxelRenderMode RenderMode = EVoxelRenderMode::Blocky;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
	int32 ChunkSizeX = 16;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
	int32 ChunkSizeY = 16;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
	int32 ChunkSizeZ = 16;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
	float VoxelSize = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	UProceduralMeshComponent* Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel|Material")
	UMaterialInterface* VoxelMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel|Data")
	TArray<FVoxelData> Voxels;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel|Data")
	TArray<FVoxelData> MarchingVoxels;

private:

	FChunkMeshData MeshData;

	UPROPERTY()
	TWeakObjectPtr<AVoxelWorld> OwningVoxelWorld;

	void InitializeVoxels();
	void InitializeMarchingVoxels();
	void EnsureVoxelDataInitialized();
	bool HasValidVoxelData() const;
	bool HasValidMarchingVoxelData() const;
	void UpdateChunkVoxelOffsetFromLocation();

	int32 GetVoxelIndex(int32 X, int32 Y, int32 Z) const;
	int32 GetMarchingVoxelIndex(int32 X, int32 Y, int32 Z) const;

	bool IsCoordinateValid(int32 X, int32 Y, int32 Z) const;
	bool IsMarchingCoordinateValid(int32 X, int32 Y, int32 Z) const;

	bool IsVoxelSolid(int32 X, int32 Y, int32 Z) const;

	void GenerateMesh();
	void GenerateBlockyMesh();
	void GenerateMarchingMesh();

	FVoxelData GetMarchingSample(int32 X, int32 Y, int32 Z) const;
	void SetMarchingSample(FIntVector LocalCoords, const FVoxelData& VoxelData);

	FVoxelData MakeVoxelData(EVoxelBlockType BlockType) const;
};


