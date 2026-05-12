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

void AEnemyTankSelfDestruct::DecideAction()
{
    if (IsInAttackRange())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 플레이어 인접! 자폭 시퀀스 가동"), *GetName());
        Explode();
    }
    else if (TurnActionCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 플레이어 추격 중... 남은 이동 횟수: %d"), *GetName(), TurnActionCount);
        MoveOnVoxelGrid();
        --TurnActionCount;
    }
    else
    {
        OnTurnEnd();
    }
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

    // 2. VoxelWorld 지형 파괴 연동 (포탄 폭발 로직 재활용)
    if (VWorld)
    {
        VWorld->HandleProjectileExplosion(ExplodeLocation, ExplosionRadius);

        // 시각적 피드백 (필요 시 블루프린트에서 설정한 이펙트 재생)
    }

    UE_LOG(LogTemp, Error, TEXT("[%s] 자폭 완료!"), *GetName());

    OnTurnEnd();
    Destroy();
}