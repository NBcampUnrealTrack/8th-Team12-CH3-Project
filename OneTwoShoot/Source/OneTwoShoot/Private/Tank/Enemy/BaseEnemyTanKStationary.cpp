#include "../Public/Tank/Enemy/BaseEnemyTanKStationary.h"
#include "GameFramework/CharacterMovementComponent.h"

ABaseEnemyTanKStationary::ABaseEnemyTanKStationary()
{
    AttackRange = 2000.f;
    MaxTurnActionCount = 1;
    TurnActionCount = MaxTurnActionCount;
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
    Super::OnTurnStart();

    UE_LOG(LogTemp, Warning, TEXT("[%s] Stationary OnTurnStart 호출됨"), *GetName());
}

void ABaseEnemyTanKStationary::MoveOnVoxelGrid()
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] 고정형 요새는 이동 연산을 스킵하고 즉시 차례를 마칩니다."), *GetName());
    OnTurnEnd();
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