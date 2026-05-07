#include "../Public/Tank/BaseProjectile.h"
#include "../Public/Tank/Enemy/EnemyAIMobileBossA.h"
#include "Kismet/KismetMathLibrary.h"

AEnemyAIMobileBossA::AEnemyAIMobileBossA()
{
    FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
    FirePoint->SetupAttachment(RootComponent);

    ShootPower = 2000.f;
}

void AEnemyAIMobileBossA::BeginPlay()
{
    Super::BeginPlay();
}

// 부모의 패턴 실행을 오버라이드
void AEnemyAIMobileBossA::ExecutePattern()
{
    if (!TargetPlayer)
    {
        OnTurnEnd();
        return;
    }

    // 1. 공격 전 플레이어를 바라봄
    Aim();

    // 2. 현재 페이즈(0, 1, 2)에 따라 분기 처리
    UE_LOG(LogTemp, Warning, TEXT("[%s] 보스 A 패턴 실행 - 페이즈 %d"), *GetName(), CurrentPhase);

    switch (CurrentPhase)
    {
    case 0:
        // 체력 70% 이상: 기본 1발 사격
        Pattern_SingleShot();
        break;
    case 1:
        // 체력 40% ~ 70%: 3갈래 확산 사격
        Pattern_SpreadShot(3, 15.f);
        break;
    case 2:
        // 체력 40% 미만 (발악기): 5갈래 넓은 확산 사격
        Pattern_SpreadShot(5, 20.f);
        break;
    default:
        Pattern_SingleShot();
        break;
    }

    OnTurnEnd();
}

void AEnemyAIMobileBossA::Aim()
{
    FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
    FRotator LookAt = Direction.Rotation();
    // Z축(Yaw)만 회전하여 플레이어를 바라보게 함
    SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
}

void AEnemyAIMobileBossA::Pattern_SingleShot()
{
    // 정면으로 1발 발사
    FireProjectile(GetActorRotation());
    UE_LOG(LogTemp, Warning, TEXT("보스 A 1발 발사!"));
}

void AEnemyAIMobileBossA::Pattern_SpreadShot(int32 ProjectileCount, float SpreadAngle)
{
    // 발사할 총 개수에 맞춰 각도를 분배하여 부채꼴로 사격
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

void AEnemyAIMobileBossA::FireProjectile(FRotator SpawnRotation)
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