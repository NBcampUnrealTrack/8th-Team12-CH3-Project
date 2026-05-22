#include "../Public/Tank/Enemy/EnemyTankMobileBossA.h"
#include "../Public/Tank/BaseProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AEnemyTankMobileBossA::AEnemyTankMobileBossA()
{
    LaunchPower = 4000.f;
}

void AEnemyTankMobileBossA::BeginPlay()
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

void AEnemyTankMobileBossA::Fire()
{
    if (!TargetPlayer)
    {
        return;
    }

    Aim();

    UE_LOG(LogTemp, Warning, TEXT("[%s] 보스 A 패턴 실행 - 페이즈 %d"), *GetName(), CurrentPhase);

    switch (CurrentPhase)
    {
    case 0:
        Pattern_SingleShot();
        break;
    case 1:
        Pattern_SpreadShot(3, 5.f);
        break;
    case 2:
        Pattern_SpreadShot(5, 5.f);
        break;
    default:
        Pattern_SingleShot();
        break;
    }
}

void AEnemyTankMobileBossA::Aim()
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

void AEnemyTankMobileBossA::Pattern_SingleShot()
{
    FireProjectile(GetActorRotation());
    UE_LOG(LogTemp, Warning, TEXT("보스 A 1발 발사!"));
}

void AEnemyTankMobileBossA::Pattern_SpreadShot(int32 ProjectileCount, float SpreadAngle)
{
    float StartAngle = -SpreadAngle * ((ProjectileCount - 1) / 2.0f);

    for (int32 i = 0; i < ProjectileCount; i++)
    {
        float CurrentYawOffset = StartAngle + (SpreadAngle * i);
        FRotator SpawnRotation = GetActorRotation();
        SpawnRotation.Yaw += CurrentYawOffset;

        FireProjectile(SpawnRotation);
    }
    UE_LOG(LogTemp, Warning, TEXT("보스 A [%d]발 발사!"), ProjectileCount);
}

void AEnemyTankMobileBossA::FireProjectile(FRotator SpawnRotation)
{
    if (!ProjectileClass || !FirePivotComp) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;

    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

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
    }
}