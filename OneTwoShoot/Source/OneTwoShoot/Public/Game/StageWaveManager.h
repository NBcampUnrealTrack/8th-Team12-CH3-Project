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

public:
    // 스테이지 구성 (최대 3개 웨이브 권장)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Config")
    TArray<FWaveData> StageWaves;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage State")
    int32 CurrentWaveIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Config")
    float SpawnDropHeight = 2000.f; // 하늘에서 떨어질 높이

    void StartNextWave(); // 다음 웨이브 시작
    void OnEnemyDestroyed(); // 적 처치 시 호출될 함수

private:
    int32 CurrentWaveEnemyCount = 0; // 현재 웨이브의 남은 적 수
};