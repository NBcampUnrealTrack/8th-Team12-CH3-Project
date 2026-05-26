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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Config")
	UDataTable* StageDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Config")
	FName TargetStageRowName;

	TArray<FWaveData> ActiveStageWaves;

	int32 CurrentWaveIndex = 0;
	int32 CurrentWaveEnemyCount = 0;

	FTimerHandle BattleStartTimerHandle;

	bool bIsWaveStartPending = false;
	bool bIsWaveActive = false;

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Config")
    float SpawnDropHeight = 2000.f;

    void StartNextWave();
    void OnEnemyDestroyed();
};