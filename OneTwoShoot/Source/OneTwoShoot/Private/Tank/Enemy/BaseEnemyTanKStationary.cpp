#include "../Public/Tank/Enemy/BaseEnemyTanKStationary.h"
#include "GameFramework/CharacterMovementComponent.h"

ABaseEnemyTanKStationary::ABaseEnemyTanKStationary()
{
    AttackRange = 2000.f;
}

void ABaseEnemyTanKStationary::BeginPlay()
{
    Super::BeginPlay();
}

//// 플레이어 방향에 따른 회전
//void ABaseEnemyTanKStationary::Aim()
//{
//    if (!TargetPlayer) return;
//
//    // 플레이어 방향 벡터 계산
//    FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
//    FRotator LookAt = Direction.Rotation();
//
//    // Yaw(좌우)만 회전, Pitch(상하)는 포신에서 처리
//    SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
//    CurrentAimAngle = LookAt.Pitch;
//}

// 턴 시작
void ABaseEnemyTanKStationary::OnTurnStart()
{
    if (bIsDead) return;

    DecideAction();
}

void ABaseEnemyTanKStationary::DecideAction()
{
    if (IsInAttackRange())
    {
        Aim();
        Fire();

        // 행동 연출 시간을 위해 1.5초 지연 후 종료
        FTimerHandle ActionDelayHandle;
        GetWorldTimerManager().SetTimer(ActionDelayHandle, this, &ABaseEnemyTank::OnTurnEnd, 1.5f, false);
    }
    else
    {
        // 사거리 밖이면 즉시 종료 (고정형은 이동 안 함)
        OnTurnEnd();
    }
}

void ABaseEnemyTanKStationary::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    // 땅에 닿았으니 그때 이동을 완전히 비활성화합니다.
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
        UE_LOG(LogTemp, Warning, TEXT("[%s] 바닥에 착지하여 고정 포탑 모드로 전환되었습니다."), *GetName());
    }
}

void ABaseEnemyTanKStationary::Fire()
{

}