#include "../Public/Tank/Enemy/EnemyTankMobileA.h"
#include "../Public/Tank/BaseProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyTankMobileA::AEnemyTankMobileA()
{
    FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
    FirePoint->SetupAttachment(RootComponent);
    AttackRange = 1500.f;
    LaunchPower = 4000.f;
}

void AEnemyTankMobileA::BeginPlay()
{
    Super::BeginPlay();
}

//void AEnemyTankMobileA::DecideAction()
//{
//    Super::DecideAction();
//}

//void AEnemyTankMobileA::Aim() 
//{
//    if (!TargetPlayer) return;
//
//    FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
//    FRotator LookAt = Direction.Rotation();
//    SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
//    CurrentAimAngle = LookAt.Pitch;
//
//    UE_LOG(LogTemp, Warning, TEXT("[%s] 조준 완료 - Yaw: %f, Pitch: %f"),
//        *GetName(), LookAt.Yaw, LookAt.Pitch);
//}

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