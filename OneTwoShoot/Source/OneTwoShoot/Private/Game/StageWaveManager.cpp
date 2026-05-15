#include "Game/StageWaveManager.h"
#include "Game/TurnGameMode.h"
#include "World/VoxelWorld.h"
#include "Tank/Enemy/BaseEnemyTank.h"
#include "Kismet/GameplayStatics.h"

AStageWaveManager::AStageWaveManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AStageWaveManager::BeginPlay()
{
    Super::BeginPlay();
    StartNextWave(); // 게임 시작 시 첫 웨이브 가동
}

void AStageWaveManager::StartNextWave()
{
    if (CurrentWaveIndex >= StageWaves.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("모든 웨이브 완료! 스테이지 클리어."));
        return;
    }

    FWaveData& Wave = StageWaves[CurrentWaveIndex];
    AVoxelWorld* VWorld = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass()));

    CurrentWaveEnemyCount = Wave.EnemiesToSpawn.Num();

    for (const FEnemySpawnInfo& Info : Wave.EnemiesToSpawn)
    {
        if (Info.EnemyClass && VWorld)
        {
            // 1. 복쉘 좌표를 월드 좌표로 변환 (Z값은 높은 곳으로 설정)
            FIntVector VoxelCoords(Info.SpawnVoxelXY.X, Info.SpawnVoxelXY.Y, 0);
            FVector SpawnPos = VWorld->VoxelToWorldLocation(VoxelCoords);
            SpawnPos.Z = SpawnDropHeight;

            // 2. 적 탱크 스폰
            GetWorld()->SpawnActor<ABaseEnemyTank>(Info.EnemyClass, SpawnPos, FRotator::ZeroRotator);
        }
    }

    // 3. 적들이 착지할 시간을 준 뒤 턴 시스템 시작
    FTimerHandle BattleStartTimer;
    GetWorldTimerManager().SetTimer(BattleStartTimer, FTimerDelegate::CreateLambda([this]() {
        if (ATurnGameMode* GM = Cast<ATurnGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
        {
            GM->StartWave();
        }
        }), 2.5f, false);
}

void AStageWaveManager::OnEnemyDestroyed()
{
    CurrentWaveEnemyCount--;

    if (CurrentWaveEnemyCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("웨이브 %d 섬멸 완료!"), CurrentWaveIndex + 1);
        CurrentWaveIndex++;

        // 잠시 후 다음 웨이브 시작
        FTimerHandle NextWaveTimer;
        GetWorldTimerManager().SetTimer(NextWaveTimer, this, &AStageWaveManager::StartNextWave, 3.0f, false);
    }
}