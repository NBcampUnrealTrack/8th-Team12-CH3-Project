// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/OverlapResult.h"
#include "VoxelDefinitions.h"
#include "VoxelWorld.generated.h"

class AVoxelChunkActor;
struct FOverlapResult;

UCLASS()
class ONETWOSHOOT_API AVoxelWorld : public AActor
{
	GENERATED_BODY()
	
public:	
	// 월드 매니저 액터 기본값 설정. Tick은 사용하지 않는다.
	AVoxelWorld();

	// 현재 레벨에 배치된 모든 VoxelChunkActor를 다시 찾아 Chunks 목록을 재구성한다.
	// 에디터에서 청크를 추가/삭제했거나 런타임에 목록이 틀어졌을 때 호출한다.
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void RefreshChunkList();

	// 특정 청크를 월드 관리 목록에 등록하고, 청크가 이 VoxelWorld를 역참조하도록 연결한다.
	// 새 청크 생성 시 또는 RefreshChunkList 내부에서 사용된다.
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void RegisterChunk(AVoxelChunkActor* Chunk);

	// 특정 청크를 월드 관리 목록에서 제거하고, 청크의 OwningVoxelWorld 연결도 끊는다.
	// 청크 제거/파괴 시 참조가 남지 않게 하기 위한 정리 함수다.
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void UnregisterChunk(AVoxelChunkActor* Chunk);

	// 월드 좌표 기준 폭발/파괴 요청을 받아, 반경과 겹치는 청크들에게 지형 파괴를 전달한다.
	// 실제 복셀 데이터 수정과 메쉬 재생성은 각 VoxelChunkActor가 수행한다.
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	void DestroyVoxelsAtWorldLocation(FVector WorldLocation, float Radius);

	// 청크가 RadialDamage를 받았을 때 월드에 폭발 파괴 처리를 요청하는 진입점.
	// 여러 청크가 같은 폭발 데미지를 동시에 보고할 수 있으므로 짧은 시간 중복 요청을 필터링한다.
	void RequestVoxelExplosionFromChunk(
		AVoxelChunkActor* ReportingChunk,
		FVector ExplosionOrigin,
		float ExplosionRadius,
		AActor* DamageCauser
	);

	// 디버그용 파괴 실행 버튼. 현재는 임의 청크 하나에서 랜덤 폭발을 일으킨다.
	UFUNCTION(BlueprintCallable, Category = "Voxel|World|Debug")
	void DebugDestroyAllChunks();

	// 등록된 청크 중 하나를 골라 임의 위치에 DebugDestroyRadius 크기의 파괴를 적용한다.
	UFUNCTION(BlueprintCallable, Category = "Voxel|World|Debug")
	void DebugDestroyRandomChunk();

	// 디버그 파괴에 사용할 반경. DebugDestroyRandomChunk에서 폭발 반경으로 사용된다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|World|Debug")
	float DebugDestroyRadius = 200.f;

	// RebuildAllChunks 호출 시 모든 청크에 적용할 렌더링 방식.
	// Blocky는 정육면체, Marching은 고정 중앙 샘플 기반 Marching Cubes를 의미한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Render")
	EVoxelRenderMode RenderMode = EVoxelRenderMode::Blocky;

	// 현재 VoxelWorld의 렌더 설정을 모든 청크에 적용하고 메쉬를 다시 생성한다.
	// 에디터에서 RenderMode를 바꾼 뒤 누르는 용도다.
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Voxel|Actions")
	void RebuildAllChunks();

	// 특정 복셀 좌표에 Pawn 계열 액터가 서 있는지 검사한다.
	// 이동 경로 탐색에서 다른 탱크/캐릭터와 겹치는 칸을 피하기 위해 사용한다.
	bool IsVoxelOccupied(FIntVector Coords, AActor* IgnoreActor = nullptr);

	// 특정 복셀 좌표가 이동 가능한 칸인지 판정한다.
	// 아래에는 지형이 있고, 현재 칸은 비어 있으며, 다른 Pawn이 점유하지 않아야 한다.
	bool IsWalkable(FIntVector Coords, AActor* IgnoreActor = nullptr, FIntVector TargetCoords = FIntVector(-99999));

	// 전역 복셀 좌표를 기준으로 해당 위치의 블록 타입만 조회한다.
	// 이동 판정처럼 셀 단위 타입만 필요한 곳에서 사용한다.
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	EVoxelBlockType GetVoxelTypeAt(FIntVector GlobalCoords);

	// 전역 복셀 좌표를 기준으로 복셀 데이터를 조회한다.
	// Marching Cubes가 청크 경계 샘플을 이웃 청크에서 읽기 위해 사용한다.
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	FVoxelData GetVoxelDataAt(FIntVector GlobalCoords) const;

	// 월드 좌표를 복셀 그리드 좌표로 변환한다.
	// VoxelSize로 나눈 뒤 내림 처리하여 해당 월드 위치가 속한 칸을 구한다.
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	FIntVector WorldToVoxelCoords(FVector WorldLocation) const;

	// 복셀 그리드 좌표를 월드 좌표로 변환한다.
	// 칸의 모서리가 아니라 중심점을 반환한다.
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	FVector VoxelToWorldLocation(FIntVector VoxelCoords) const;

	// 임의의 월드 위치에서 가장 가까운 보행 가능 표면 좌표를 찾는다.
	// 캐릭터가 땅속에 있으면 위로, 허공에 있으면 아래로 훑는다.
	UFUNCTION(BlueprintCallable, Category = "Voxel|World")
	FIntVector GetNearestWalkableVoxel(FVector WorldLocation);

	// 월드 단위에서 사용하는 복셀 한 변의 길이.
	// 현재 ChunkActor의 VoxelSize와 자동 동기화되지 않으므로 값 불일치에 주의해야 한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|World")
	float VoxelSize = 100.0f;

protected:
	// 게임 시작 시 레벨에 배치된 청크들을 찾아 관리 목록을 초기화한다.
	virtual void BeginPlay() override;

	// 현재 VoxelWorld가 관리하는 청크 목록.
	// 파괴 전달, 전체 재빌드, 전역 좌표 조회에서 순회한다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel|World")
	TArray<TObjectPtr<AVoxelChunkActor>> Chunks;

	// 마지막으로 처리한 폭발의 DamageCauser.
	// 중복 폭발 요청 필터링과 디버그 로그용으로 보관한다.
	UPROPERTY()
	TWeakObjectPtr<AActor> LastExplosionDamageCauser;

	// 마지막 폭발 요청 시간. 같은 폭발이 여러 청크에서 중복 보고되는 것을 막기 위해 사용한다.
	float LastExplosionTime = -1.0f;

	// 마지막 폭발 위치. 새 요청이 같은 폭발인지 비교하는 기준이다.
	FVector LastExplosionOrigin = FVector::ZeroVector;

	// 마지막 폭발 반경. 위치/시간과 함께 중복 폭발 요청 판정에 사용한다.
	float LastExplosionRadius = 0.0f;
};
