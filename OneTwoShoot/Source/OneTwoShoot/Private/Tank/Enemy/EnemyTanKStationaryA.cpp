#include "Tank/Enemy/EnemyTanKStationaryA.h"
#include "Tank/BaseProjectile.h"
#include "Kismet/GameplayStatics.h"

AEnemyTanKStationaryA::AEnemyTanKStationaryA()
{
    AttackRange = 3000.f;
    MaxChargeTurns = 2; // 예: 2턴 주기 사격
    CurrentChargeTurns = 0;

    LaunchPower = 6000.f;
}

void AEnemyTanKStationaryA::BeginPlay()
{
    Super::BeginPlay();

    TArray<USceneComponent*> SceneComps;
    GetComponents<USceneComponent>(SceneComps);

    for (USceneComponent* CurrComp : SceneComps)
    {
        if (CurrComp)
        {
            if (CurrComp->GetName() == TEXT("BarrelPivot"))
            {
                BarrelPivotComp = CurrComp;
            }
            else if (CurrComp->GetName() == TEXT("FirePoint"))
            {
                FirePivotComp = CurrComp;
            }
        }
    }
}

void AEnemyTanKStationaryA::OnTurnStart()
{
    if (bIsDead) return;

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

void AEnemyTanKStationaryA::Aim()
{
    Super::Aim();

    if (BarrelPivotComp && FirePivotComp && TargetPlayer)
    {
        FVector TossVelocity;
        bool bSuccess = UGameplayStatics::SuggestProjectileVelocity(
            this,
            TossVelocity,
            FirePivotComp->GetComponentLocation(),
            TargetPlayer->GetActorLocation(),
            LaunchPower,
            false, 0.f, 0.f,
            ESuggestProjVelocityTraceOption::DoNotTrace
        );

        if (bSuccess)
        {
            FRotator PerfectAimRot = TossVelocity.Rotation();
            FRotator BarrelRelRot = BarrelPivotComp->GetRelativeRotation();

            float TargetPitch = PerfectAimRot.Pitch - GetActorRotation().Pitch;
            TargetPitch = FMath::Clamp(TargetPitch, -35.0f, 50.0f);

            BarrelRelRot.Pitch = TargetPitch;
            BarrelPivotComp->SetRelativeRotation(BarrelRelRot);
        }
    }
}

void AEnemyTanKStationaryA::Fire()
{
    if (!ProjectileClass || !FirePivotComp) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    FRotator SpawnRotation = GetActorRotation();
    SpawnRotation.Pitch = CurrentAimAngle;

    ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(
        ProjectileClass,
        FirePivotComp->GetComponentLocation(),
        SpawnRotation,
        SpawnParams
    );

    if (Projectile)
    {
        FVector LaunchDirection = SpawnRotation.Vector();
        Projectile->FireInDirection(LaunchDirection, LaunchPower, FVector::ZeroVector);
        UE_LOG(LogTemp, Warning, TEXT("[%s] 포탄 발사 성공!"), *GetName());
    }
}

bool AEnemyTanKStationaryA::IsChargedTurn()
{
    return CurrentChargeTurns >= MaxChargeTurns;
}