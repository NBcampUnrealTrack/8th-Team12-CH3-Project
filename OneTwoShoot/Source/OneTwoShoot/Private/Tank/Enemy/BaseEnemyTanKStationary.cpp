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

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 0.f;
        UE_LOG(LogTemp, Warning, TEXT("[%s] 바닥에 착지하여 고정 포탑 모드로 전환되었습니다."), *GetName());
    }
}

void ABaseEnemyTanKStationary::Fire()
{

}