#include "../Public/Tank/Enemy/BaseEnemyTankMobile.h"
#include "../Public/World/VoxelPathfinder.h"
#include "../Public/World/VoxelWorld.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

ABaseEnemyTankMobile::ABaseEnemyTankMobile()
{
    bIsMoving = false;

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
}

void ABaseEnemyTankMobile::BeginPlay()
{
    Super::BeginPlay();

    VWorld = GetVoxelWorld();

    if (!VWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] VoxelWorld를 찾을 수 없습니다!"), *GetName());
    }

    if (!TargetPlayer)
    {
        TargetPlayer = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

        if (TargetPlayer)
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] 타겟 플레이어(%s)를 성공적으로 찾았습니다."),
                *GetName(), *TargetPlayer->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[%s] 플레이어 Pawn을 찾았으나 ACharacter 타입이 아닙니다!"), *GetName());
        }
    }
}

void ABaseEnemyTankMobile::OnTurnStart()
{
    Super::OnTurnStart();

    UE_LOG(LogTemp, Warning, TEXT("[%s] Mobile OnTurnStart 호출됨"), *GetName());
    DecideAction();
}

void ABaseEnemyTankMobile::MoveOnVoxelGrid()
{
    if (!VWorld || !TargetPlayer) return;

    FVector StartLocation = GetActorLocation();
    FVector TargetLocation = TargetPlayer->GetActorLocation();

    FIntVector StartVoxel = VWorld->GetNearestWalkableVoxel(StartLocation);
    FIntVector EndVoxel = VWorld->GetNearestWalkableVoxel(TargetLocation);

    TArray<FIntVector> FullPath = VoxelPathfinder::FindPath(VWorld, StartVoxel, EndVoxel, 100, this);

    if (FullPath.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] 경로 탐색 실패!"), *GetName());
        bIsMoving = false;
        OnTurnEnd();
        return;
    }

    // 전체 경로 중 '다음 한 칸'만 담아서 보냅니다.
    TArray<FIntVector> NextStepPath;
    NextStepPath.Add(FullPath[0]);

    UE_LOG(LogTemp, Warning, TEXT("[%s] 경로 찾기 성공: %d칸 이동 시작"), *GetName(), FullPath.Num());
    bIsMoving = true;
    ExecuteVoxelMovement(NextStepPath);
}

void ABaseEnemyTankMobile::ExecuteVoxelMovement(TArray<FIntVector> Path)
{
    if (IsInAttackRange())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 이동 중 사거리 진입! 남은 경로를 포기하고 행동 결정."), *GetName());

        bIsMoving = false;
        Path.Empty();

        // 이동 애니메이션 시간을 고려해 다시 판단
        FTimerHandle MoveDelayHandle;
        GetWorldTimerManager().SetTimer(MoveDelayHandle, this, &ABaseEnemyTankMobile::DecideAction, 1.0f, false);
        return;
    }

    if (Path.Num() == 0)
    {
        bIsMoving = false;

        // 이동 애니메이션 시간을 고려해 다시 판단
        FTimerHandle MoveDelayHandle;
        GetWorldTimerManager().SetTimer(MoveDelayHandle, this, &ABaseEnemyTankMobile::DecideAction, 1.0f, false);
        return;
    }

    bIsMoving = true;
    FIntVector NextStepCoords = Path[0];

    //CurrentRemainingPath = Path;
    //CurrentRemainingPath.RemoveAt(0);

    FVector TargetWorldPos = VWorld->VoxelToWorldLocation(NextStepCoords);
    TargetWorldPos.Z += 50.0f; // VoxelSize가 100이라서 절반 정도 위로 보정

    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);

        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(TargetWorldPos);
        MoveRequest.SetAcceptanceRadius(10.0f); // 너무 작으면 도착 판정이 힘듦

        MoveRequest.SetUsePathfinding(false);

        FPathFollowingRequestResult RequestResult = AIC->MoveTo(MoveRequest);

        if (RequestResult.Code == EPathFollowingRequestResult::Failed)
        {
            UE_LOG(LogTemp, Error, TEXT("[%s] 이동 요청 실패!"), *GetName());
            bIsMoving = false;
            OnTurnEnd();
            return;
        }

        AIC->GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &ABaseEnemyTankMobile::OnMoveComplete);
    }
}

void ABaseEnemyTankMobile::OnMoveComplete(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] 이동 완료 - 성공 여부: %s"),
        *GetName(), Result.IsSuccess() ? TEXT("성공") : TEXT("실패"));

    if (Result.IsSuccess())
    {
        //ExecuteVoxelMovement(CurrentRemainingPath);
        DecideAction();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] 복쉘 이동 중단됨."), *GetName());
        bIsMoving = false;
        OnTurnEnd();
    }
}

void ABaseEnemyTankMobile::DecideAction()
{
    if (bIsDead) return;

    if (IsInAttackRange())
    {
        Aim();
        Fire();

        FTimerHandle ActionDelayHandle;
        GetWorldTimerManager().SetTimer(ActionDelayHandle, this, &ABaseEnemyTankMobile::OnTurnEnd, 1.5f, false);
    }
    else if (TurnActionCount > 0)
    {
        MoveOnVoxelGrid();
        --TurnActionCount;
    }
    else
    {
        OnTurnEnd();
    }
}

AVoxelWorld* ABaseEnemyTankMobile::GetVoxelWorld()
{
    AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass());
    return Cast<AVoxelWorld>(FoundActor);
}

void ABaseEnemyTankMobile::Fire()
{

}