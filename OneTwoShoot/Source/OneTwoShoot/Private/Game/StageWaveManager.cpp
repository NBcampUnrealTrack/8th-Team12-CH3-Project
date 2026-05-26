#include "Game/StageWaveManager.h"
#include "Game/TurnGameMode.h"
#include "World/VoxelWorld.h"
#include "Tank/Enemy/BaseEnemyTank.h"
#include "Kismet/GameplayStatics.h"
#include "Game/OneTwoShootGameInstance.h"

AStageWaveManager::AStageWaveManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AStageWaveManager::BeginPlay()
{
    Super::BeginPlay();

    if (StageDataTable)
    {
        FStageDataRow* Data = StageDataTable->FindRow<FStageDataRow>(TargetStageRowName, TEXT(""));
        if (Data)
        {
            ActiveStageWaves = Data->Waves;
            UE_LOG(LogTemp, Warning, TEXT("%s 데이터를 성공적으로 로드했습니다."), *TargetStageRowName.ToString());
        }
    }

    StartNextWave();
}

void AStageWaveManager::StartNextWave()
{
    if (bIsWaveStartPending || bIsWaveActive) return;

    if (CurrentWaveIndex >= ActiveStageWaves.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("모든 웨이브 완료! 스테이지 클리어. 다음 레벨로 이동합니다."));

        if (UOneTwoShootGameInstance* GI = Cast<UOneTwoShootGameInstance>(GetGameInstance()))
        {
            GI->AdvanceToNextStage();
        }
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("다음 웨이브"));
    bIsWaveStartPending = true;

    FWaveData& Wave = ActiveStageWaves[CurrentWaveIndex];
    AVoxelWorld* VWorld = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass()));

    CurrentWaveEnemyCount = Wave.EnemiesToSpawn.Num();

    for (const FEnemySpawnInfo& Info : Wave.EnemiesToSpawn)
    {
        if (Info.EnemyClass && VWorld)
        {

            FIntVector VoxelCoords(Info.SpawnVoxelXY.X, Info.SpawnVoxelXY.Y, 0);
            FVector SpawnPos = VWorld->VoxelToWorldLocation(VoxelCoords);
            SpawnPos.Z = SpawnDropHeight;

            GetWorld()->SpawnActor<ABaseEnemyTank>(Info.EnemyClass, SpawnPos, FRotator::ZeroRotator);
        }
    }

    GetWorldTimerManager().SetTimer(BattleStartTimerHandle, FTimerDelegate::CreateLambda([this]() {
        bIsWaveStartPending = false;
        bIsWaveActive = true;
        if (ATurnGameMode* GM = Cast<ATurnGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
        {
            GM->StartWave();
        }
        }), 2.5f, false);
}

void AStageWaveManager::OnEnemyDestroyed()
{
    if (!bIsWaveActive) return;

    CurrentWaveEnemyCount--;

    if (CurrentWaveEnemyCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("웨이브 %d 섬멸 완료!"), CurrentWaveIndex + 1);
        bIsWaveActive = false;
        CurrentWaveIndex++;

        if (ATurnGameMode* GM = Cast<ATurnGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
        {
            GM->EndWave();
        }

        // 잠시 후 다음 웨이브 시작
        FTimerHandle NextWaveTimer;
        GetWorldTimerManager().SetTimer(NextWaveTimer, this, &AStageWaveManager::StartNextWave, 3.0f, false);
    }
}