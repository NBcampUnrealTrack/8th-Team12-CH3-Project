#include "Tank/Enemy/EnemyTanKStationaryA.h"
#include "Tank/BaseProjectile.h"
#include "Kismet/GameplayStatics.h"

AEnemyTanKStationaryA::AEnemyTanKStationaryA()
{
    FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
    FirePoint->SetupAttachment(RootComponent);

    AttackRange = 3000.f;
    MaxChargeTurns = 1;
    CurrentChargeTurns = 0;
}

void AEnemyTanKStationaryA::OnTurnStart()
{
    // 부모 클래스의 OnTurnStart를 부르면 즉시 Fire()가 호출되므로 Super::OnTurnStart() 대신 개별 구현함.

    if (bIsDead) return;

    if (IsInAttackRange())
    {
        CurrentChargeTurns++;

        if (IsChargedTurn())
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] 포격 준비 완료 - 포격 실행!"), *GetName());
            Aim();
            Fire();
            CurrentChargeTurns = 0;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] 포격 준비 중... (%d/%d)"),
                *GetName(), CurrentChargeTurns, MaxChargeTurns);

        }
    }

    OnTurnEnd();
}

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
    }
}

bool AEnemyTanKStationaryA::IsChargedTurn()
{
    return CurrentChargeTurns >= MaxChargeTurns ? true : false;
}