#include "EnemyAIMobile.h"
#include "AI/Navigation/NavigationTypes.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

AEnemyAIMobile::AEnemyAIMobile()
{
    MoveRange = 500.f;
    bIsMoving = false;

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
}

void AEnemyAIMobile::BeginPlay()
{
    Super::BeginPlay();
}

void AEnemyAIMobile::OnTurnStart()
{
    Super::OnTurnStart();

    UE_LOG(LogTemp, Warning, TEXT("[%s] Mobile OnTurnStart 호출됨"), *GetName());
    DecideAction();
}

void AEnemyAIMobile::Move()
{
    if (!TargetPlayer) return;

    AAIController* AIC = Cast<AAIController>(GetController());
    if (!AIC) return;

    // 바인딩 전에 항상 먼저 해제 (중복 방지)
    AIC->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);

    // 플레이어 방향으로 MoveRange만큼 이동
    FVector Direction = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector TargetLocation = GetActorLocation() + Direction * MoveRange;

    bIsMoving = true;

    // 이동 완료 시 OnMoveComplete 콜백
    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(TargetLocation);
    MoveRequest.SetAcceptanceRadius(10.f);

    FNavPathSharedPtr NavPath;
    AIC->MoveTo(MoveRequest, &NavPath);

    // 이동 완료 델리게이트 바인딩
    AIC->GetPathFollowingComponent()->OnRequestFinished.AddUObject(
        this, &AEnemyAIMobile::OnMoveComplete
    );


    UE_LOG(LogTemp, Warning, TEXT("[%s] 이동 시작 - 목표 위치: %s"),
        *GetName(), *TargetLocation.ToString());
}

void AEnemyAIMobile::OnMoveComplete(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] 이동 완료 - 성공 여부: %s"),
        *GetName(), Result.IsSuccess() ? TEXT("성공") : TEXT("실패"));

    bIsMoving = false;

    // 델리게이트 해제
    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
    }

    if (Result.IsSuccess())
    {
        // 이동 성공 시 다음 행동 결정 (재이동 혹은 공격)
        DecideAction();
    }
    else
    {
        // 이동 실패 시(예: 경로 막힘) 억지로라도 턴을 종료해야 게임이 멈추지 않음
        UE_LOG(LogTemp, Error, TEXT("[%s] 이동 실패로 인해 강제로 턴을 종료합니다."), *GetName());
        OnTurnEnd();
    }
}

void AEnemyAIMobile::DecideAction()
{
    // 하위 클래스에서 구현
    // ex) 자폭형 → 근접 여부 확인 후 폭발
    //     이동형 포격 → Aim() → Fire()
}