#include "Tank/Enemy/EnemyTankSelfDestruct.h"
#include "World/VoxelWorld.h"
#include "Kismet/GameplayStatics.h"

AEnemyTankSelfDestruct::AEnemyTankSelfDestruct()
{
    AttackRange = 200.f;
    ExplosionDamage = 150.f;
    ExplosionRadius = 400.f;
    MaxHealth = 100.f;
    CurrentHealth = MaxHealth;

    UE_LOG(LogTemp, Warning, TEXT("[%s] 자폭 탱크 생성 완료"), *GetName());
}

//void AEnemyTankSelfDestruct::DecideAction()
//{
//    Super::DecideAction();
//}

void AEnemyTankSelfDestruct::Fire()
{
    Explode();
}

void AEnemyTankSelfDestruct::Explode()
{
    FVector ExplodeLocation = GetActorLocation();

    UGameplayStatics::ApplyRadialDamage(
        this,
        ExplosionDamage,
        ExplodeLocation,
        ExplosionRadius,
        UDamageType::StaticClass(),
        TArray<AActor*>(),
        this,
        GetInstigatorController(),
        true
    );

    UE_LOG(LogTemp, Error, TEXT("[%s] 자폭 완료!"), *GetName());

    OnTurnEnd();
    SetLifeSpan(0.1f);
}