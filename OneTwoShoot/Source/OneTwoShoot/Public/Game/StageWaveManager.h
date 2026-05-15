#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TankGameTypes.h"
#include "StageWaveManager.generated.h"

UCLASS()
class ONETWOSHOOT_API AStageWaveManager : public AActor
{
    GENERATED_BODY()

public:
    AStageWaveManager();

protected:
    virtual void BeginPlay() override;

	// 에디터에서 할당할 데이터 테이블 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Config")
	UDataTable* StageDataTable;

	// 현재 어떤 스테이지를 불러올지 결정하는 키값 (Row Name)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Config")
	FName TargetStageRowName;

	// 테이블에서 불러온 실제 웨이브 데이터들을 담아둘 내부 배열
	TArray<FWaveData> ActiveStageWaves;

	int32 CurrentWaveIndex = 0;
	int32 CurrentWaveEnemyCount = 0;

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Config")
    float SpawnDropHeight = 2000.f; // 하늘에서 떨어질 높이

    void StartNextWave(); // 다음 웨이브 시작
    void OnEnemyDestroyed(); // 적 처치 시 호출될 함수
};