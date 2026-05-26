#include "../Public/Tank/Enemy/BaseEnemyTankMobile.h"
#include "../Public/World/VoxelPathfinder.h"
#include "../Public/World/VoxelWorld.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

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

    if (FullPath.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] 경로가 0개입니다. 주행 불가능 상태이므로 안전하게 다음 행동력 판단으로 패스."), *GetName());
        bIsMoving = false;

        GetWorldTimerManager().SetTimerForNextTick(this, &ABaseEnemyTankMobile::DecideAction);
        return;
    }

    TArray<FIntVector> NextStepPath;

    if (FullPath.Num() > 1)
    {
        NextStepPath.Add(FullPath[1]);
        UE_LOG(LogTemp, Warning, TEXT("[%s] 안전 경로 확보: 총 %d칸 중 실제 전진할 앞 칸(인덱스 1) 지정"), *GetName(), FullPath.Num());
    }
    else
    {
        NextStepPath.Add(FullPath[0]);
        UE_LOG(LogTemp, Warning, TEXT("[%s] 최종 목적지 인접: 인덱스 0 지정"), *GetName());
    }

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

    FVector RayStart = TargetWorldPos + FVector(0.f, 0.f, 200.f);
    FVector RayEnd = TargetWorldPos - FVector(0.f, 0.f, 200.f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_WorldDynamic, QueryParams))
    {
        TargetWorldPos.Z = HitResult.ImpactPoint.Z;
    }
    else
    {
        TargetWorldPos.Z += 50.0f;
    }

    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);

        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(TargetWorldPos);
        MoveRequest.SetAcceptanceRadius(35.0f);
        MoveRequest.SetUsePathfinding(false);
        MoveRequest.SetReachTestIncludesAgentRadius(false);

        FPathFollowingRequestResult RequestResult = AIC->MoveTo(MoveRequest);

        if (MoveSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                MoveSound,
                GetActorLocation()
            );
        }

        if (RequestResult.Code == EPathFollowingRequestResult::Failed ||
            RequestResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] MoveTo가 즉시 종료됨 (코드: %d). 안전하게 다음판단 시퀀스로 토스."), *GetName(), (int32)RequestResult.Code);
            bIsMoving = false;

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

AVoxelWorld* ABaseEnemyTankMobile::GetVoxelWorld()
{
    AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass());
    return Cast<AVoxelWorld>(FoundActor);
}

void ABaseEnemyTankMobile::Fire()
{

}