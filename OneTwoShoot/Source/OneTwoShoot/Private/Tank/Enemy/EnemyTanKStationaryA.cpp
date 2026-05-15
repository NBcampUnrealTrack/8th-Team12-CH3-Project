#include "Tank/Enemy/EnemyTanKStationaryA.h"
#include "Tank/BaseProjectile.h"
#include "Kismet/GameplayStatics.h"

AEnemyTanKStationaryA::AEnemyTanKStationaryA()
{
    FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
    FirePoint->SetupAttachment(RootComponent);

    AttackRange = 3000.f;
    MaxChargeTurns = 2; // 예: 2턴 주기 사격
    CurrentChargeTurns = 0;
}

void AEnemyTanKStationaryA::OnTurnStart()
{
    if (bIsDead) return;

    // 로직을 DecideAction으로 일원화
    DecideAction();
}

void AEnemyTanKStationaryA::DecideAction()
{
    if (IsInAttackRange())
    {
        CurrentChargeTurns++;

        if (CurrentChargeTurns >= MaxChargeTurns)
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] 충전 완료! 사격합니다."), *GetName());
            Aim();
            Fire();
            CurrentChargeTurns = 0;

            FTimerHandle ActionDelayHandle;
            GetWorldTimerManager().SetTimer(ActionDelayHandle, this, &ABaseEnemyTank::OnTurnEnd, 1.5f, false);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] 충전 중... (%d/%d)"), *GetName(), CurrentChargeTurns, MaxChargeTurns);

            FTimerHandle ChargeDelayHandle;
            GetWorldTimerManager().SetTimer(ChargeDelayHandle, this, &ABaseEnemyTank::OnTurnEnd, 1.0f, false);
        }
    }
    else
    {
        OnTurnEnd();
    }
}

// 사격 방식 구체화 (오버라이드)
void AEnemyTanKStationaryA::Fire()
{
    if (!ProjectileClass || !FirePoint) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    FRotator SpawnRotation = GetActorRotation();
    SpawnRotation.Pitch = CurrentAimAngle;

    ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(
        ProjectileClass,
        FirePoint->GetComponentLocation(),
        SpawnRotation,
        SpawnParams
    );

    if (Projectile)
    {
        FVector LaunchDirection = SpawnRotation.Vector();
        Projectile->FireInDirection(LaunchDirection, FirePower, FVector::ZeroVector);
        UE_LOG(LogTemp, Warning, TEXT("[%s] 포탄 발사 성공!"), *GetName());
    }
}

bool AEnemyTanKStationaryA::IsChargedTurn()
{
    return CurrentChargeTurns >= MaxChargeTurns;
}