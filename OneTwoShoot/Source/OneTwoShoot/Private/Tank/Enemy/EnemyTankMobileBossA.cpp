#include "../Public/Tank/Enemy/EnemyTankMobileBossA.h"
#include "../Public/Tank/BaseProjectile.h"
#include "Kismet/KismetMathLibrary.h"

AEnemyTankMobileBossA::AEnemyTankMobileBossA()
{
    FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
    FirePoint->SetupAttachment(RootComponent);

    ShootPower = 4000.f;
}

void AEnemyTankMobileBossA::BeginPlay()
{
    Super::BeginPlay();
}

void AEnemyTankMobileBossA::ExecutePattern()
{
    if (!TargetPlayer)
    {
        OnTurnEnd();
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
        Pattern_SpreadShot(3, 15.f);
        break;
    case 2:
        Pattern_SpreadShot(5, 20.f);
        break;
    default:
        Pattern_SingleShot();
        break;
    }

    OnTurnEnd();
}

void AEnemyTankMobileBossA::Aim()
{
    FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
    FRotator LookAt = Direction.Rotation();
    SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
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
    if (!ProjectileClass || !FirePoint) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(
        ProjectileClass,
        FirePoint->GetComponentLocation(),
        SpawnRotation,
        SpawnParams
    );

    if (Projectile)
    {
        FVector LaunchDirection = SpawnRotation.Vector();
        Projectile->FireInDirection(LaunchDirection, ShootPower, FVector::ZeroVector);
    }
}