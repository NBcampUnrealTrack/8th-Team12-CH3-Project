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

	UFUNCTION(BlueprintCallable, Category = "Voxel|Debug")
	void DestroyVoxelsAtWorldLocation(FVector WorldLocation, float Radius);

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

	// 청크가 시작되는 전역 복쉘 좌표
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FIntVector ChunkVoxelOffset;

	// 청크의 한 변 길이
	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 ChunkSize = 16;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(CallInEditor, Category = "Voxel") // Editor-Details-Voxel. Rebuild the Mesh for the chunk.
	void RebuildChunk();

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

	UPROPERTY()
	TArray<EVoxelBlockType> Voxels;


	void InitializeVoxels();

	int32 GetVoxelIndex(int32 X, int32 Y, int32 Z) const;

	bool IsCoordinateValid(int32 X, int32 Y, int32 Z) const;

	bool IsVoxelSolid(int32 X, int32 Y, int32 Z) const;

	void GenerateMesh();

private:
	
	FChunkMeshData MeshData;
	
	UPROPERTY()
	TWeakObjectPtr<AVoxelWorld> OwningVoxelWorld;

	void AddCube(int32 X, int32 Y, int32 Z);
	void AddFace(EVoxelDirection Direction, const FVector& Base);
	FIntVector GetDirectionOffset(EVoxelDirection Direction) const;
	FVector GetDirectionNormal(EVoxelDirection Direction) const;
	TArray<FVector> GetFaceVertices(EVoxelDirection Direction, const FVector& Base) const;
};


