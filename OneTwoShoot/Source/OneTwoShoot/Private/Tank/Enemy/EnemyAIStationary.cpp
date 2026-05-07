#include "EnemyAIStationary.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyAIStationary::AEnemyAIStationary()
{
    AttackRange = 2000.f;
    CurrentAimAngle = 0.f;

    // 이동 완전 비활성화
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }
}

void AEnemyAIStationary::BeginPlay()
{
    Super::BeginPlay();
}

// 플레이어 방향에 따른 회전
void AEnemyAIStationary::Aim()
{
    if (!TargetPlayer) return;

    // 플레이어 방향 벡터 계산
    FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
    FRotator LookAt = Direction.Rotation();

    // Yaw(좌우)만 회전, Pitch(상하)는 포신에서 처리
    SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
    CurrentAimAngle = LookAt.Pitch;
}

// 사격 로직
void AEnemyAIStationary::Fire()
{
    // 하위 클래스(AEnemyCannon 등)에서 구현
    // 투사체 스폰, 발사 등
}

// 턴 시작
void AEnemyAIStationary::OnTurnStart()
{
    Super::OnTurnStart();
    if (IsInAttackRange())
    {
        Aim();
        Fire();
    }
    OnTurnEnd();
}