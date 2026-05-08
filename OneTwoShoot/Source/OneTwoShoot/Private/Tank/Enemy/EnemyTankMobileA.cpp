

#include "../Public/Tank/Enemy/EnemyTankMobileA.h"
#include "../Public/Tank/BaseProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyTankMobileA::AEnemyTankMobileA()
{
    CurrentAimAngle = 0.f;

    FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
    FirePoint->SetupAttachment(RootComponent);
    AttackRange = 1500.f;
    LaunchPower = 4000.f;
}

void AEnemyTankMobileA::BeginPlay()
{
    Super::BeginPlay();
}

void AEnemyTankMobileA::DecideAction()
{
    if (!IsInAttackRange() && TurnActionCount>0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 사거리 밖 - 재이동, 턴 카운트 [%d]"), *GetName(), TurnActionCount);
        MoveOnVoxelGrid();
        --TurnActionCount;
    }
    else if(!IsInAttackRange() && TurnActionCount <= 0)
    {
        OnTurnEnd();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 사거리 안 - 조준 및 발사"), *GetName());
        Aim();
        Fire();
        OnTurnEnd();
    }
}

void AEnemyTankMobileA::Aim()
{
    if (!TargetPlayer) return;

    FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
    FRotator LookAt = Direction.Rotation();
    SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
    CurrentAimAngle = LookAt.Pitch;

    UE_LOG(LogTemp, Warning, TEXT("[%s] 조준 완료 - Yaw: %f, Pitch: %f"),
        *GetName(), LookAt.Yaw, LookAt.Pitch);
}

void AEnemyTankMobileA::Fire()
{
    if (!ProjectileClass) return;
    if (!FirePoint) return;

    FVector SpawnLocation = FirePoint->GetComponentLocation();
    FRotator SpawnRotation = FRotator(CurrentAimAngle, GetActorRotation().Yaw, 0.f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    UE_LOG(LogTemp, Warning, TEXT("발사!"));

    ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(
        ProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (Projectile)
    {
        FVector LaunchDirection = SpawnRotation.Vector();

        FVector WindForce = FVector::ZeroVector;

        Projectile->FireInDirection(LaunchDirection, LaunchPower, WindForce);

        UE_LOG(LogTemp, Warning, TEXT("투사체 발사 성공!"));
    }
}