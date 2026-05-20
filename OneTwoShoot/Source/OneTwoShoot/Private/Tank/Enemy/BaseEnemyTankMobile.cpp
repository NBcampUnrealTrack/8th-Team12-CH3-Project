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
}

void ABaseEnemyTankMobile::MoveOnVoxelGrid()
{
    if (!VWorld || !TargetPlayer) return;

    FVector StartLocation = GetActorLocation();
    FVector TargetLocation = TargetPlayer->GetActorLocation();

    FIntVector StartVoxel = VWorld->GetNearestWalkableVoxel(StartLocation);
    FIntVector EndVoxel = VWorld->GetNearestWalkableVoxel(TargetLocation);

    TArray<FIntVector> FullPath = VoxelPathfinder::FindPath(VWorld, StartVoxel, EndVoxel, 100, this);

    if (FullPath.Num() <= 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 더 이상 이동할 경로가 없거나 이미 타겟 복쉘에 인접해 있습니다."), *GetName());
        bIsMoving = false;
        OnTurnEnd();
        return;
    }

    TArray<FIntVector> NextStepPath;
    NextStepPath.Add(FullPath[1]);

    UE_LOG(LogTemp, Warning, TEXT("[%s] 경로 확보 성공: 총 %d칸 중 다음 1칸(인덱스 1)으로 전진 개시"), *GetName(), FullPath.Num());
    bIsMoving = true;
    ExecuteVoxelMovement(NextStepPath);
}

void ABaseEnemyTankMobile::ExecuteVoxelMovement(TArray<FIntVector> Path)
{
    if (Path.Num() == 0)
    {
        bIsMoving = false;
        DecideAction();
        return;
    }

    bIsMoving = true;
    FIntVector NextStepCoords = Path[0];

    FVector TargetWorldPos = VWorld->VoxelToWorldLocation(NextStepCoords);
    TargetWorldPos.Z += 50.0f;

    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);

        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(TargetWorldPos);
        MoveRequest.SetAcceptanceRadius(15.0f); // 팅김 방지를 위해 수치 여유 확보
        MoveRequest.SetUsePathfinding(false);

        FPathFollowingRequestResult RequestResult = AIC->MoveTo(MoveRequest);

        // ======= ★ [교정 3]: 엔진이 즉시 주행을 종료하거나 거부했을 때 데드락 방지 안전장치 =======
        if (RequestResult.Code == EPathFollowingRequestResult::Failed ||
            RequestResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] MoveTo가 즉시 종료됨 (코드: %d). 안전하게 다음판단 시퀀스로 토스."), *GetName(), (int32)RequestResult.Code);
            bIsMoving = false;

            // 무브먼트 컴포넌트가 현재 프레임을 정리할 수 있도록 한 틱 뒤에 DecideAction을 안전하게 호출합니다.
            GetWorldTimerManager().SetTimerForNextTick(this, &ABaseEnemyTankMobile::DecideAction);
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
        UE_LOG(LogTemp, Warning, TEXT("[이동 발] 🏁 %s 1칸 이동 완벽히 성공! 다시 뇌(DecideAction)로 제어권 넘김."), *GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[이동 발] ⚠️ %s 1칸 이동 중단/실패함 (지형에 걸림 등). 다시 뇌(DecideAction)로 제어권 넘김."), *GetName());
    }

    DecideAction();
}

//void ABaseEnemyTankMobile::DecideAction()
//{
//    if (bIsDead) return;
//
//    if (IsInAttackRange())
//    {
//        Aim();
//        Fire();
//
//        FTimerHandle ActionDelayHandle;
//        GetWorldTimerManager().SetTimer(ActionDelayHandle, this, &ABaseEnemyTankMobile::OnTurnEnd, 1.5f, false);
//    }
//    else if (TurnActionCount > 0)
//    {
//        MoveOnVoxelGrid();
//        --TurnActionCount;
//    }
//    else
//    {
//        OnTurnEnd();
//    }
//}

AVoxelWorld* ABaseEnemyTankMobile::GetVoxelWorld()
{
    AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass());
    return Cast<AVoxelWorld>(FoundActor);
}

void ABaseEnemyTankMobile::Fire()
{

}