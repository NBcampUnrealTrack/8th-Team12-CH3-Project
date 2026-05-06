// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyAIMobileA.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BaseProjectile.h"
#include "Kismet/GameplayStatics.h"

AEnemyAIMobileA::AEnemyAIMobileA()
{
    CurrentAimAngle = 0.f;

    // 발사 위치 컴포넌트 생성
    FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
    FirePoint->SetupAttachment(RootComponent);
    AttackRange = 1500.f;
}

void AEnemyAIMobileA::BeginPlay()
{
    Super::BeginPlay();
}

void AEnemyAIMobileA::DecideAction()
{
    if (!IsInAttackRange() && TurnActionCount>0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 사거리 밖 - 재이동, 턴 카운트 [%d]"), *GetName(), TurnActionCount);
        Move();
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

void AEnemyAIMobileA::Aim()
{
    if (!TargetPlayer) return;

    FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
    FRotator LookAt = Direction.Rotation();
    SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
    CurrentAimAngle = LookAt.Pitch;

    UE_LOG(LogTemp, Warning, TEXT("[%s] 조준 완료 - Yaw: %f, Pitch: %f"),
        *GetName(), LookAt.Yaw, LookAt.Pitch);
}

void AEnemyAIMobileA::Fire()
{
    if (!ProjectileClass) return;
    if (!FirePoint) return;

    FVector SpawnLocation = FirePoint->GetComponentLocation();
    FRotator SpawnRotation = FRotator(CurrentAimAngle, GetActorRotation().Yaw, 0.f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    UE_LOG(LogTemp, Warning, TEXT("발사!"));
    // 투사체 스폰
    ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(
        ProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (Projectile)
    {
        // 3. 발사 방향 계산 (회전값으로부터 앞방향 벡터 추출)
        FVector LaunchDirection = SpawnRotation.Vector();

        // 4. 발사 파워 설정 (블루프린트 변수로 빼는 것을 추천합니다)
        float LaunchPower = 2000.f;

        // 5. 바람의 영향 (현재는 제로 벡터, 필요 시 스테이지 데이터에서 가져옴)
        FVector WindForce = FVector::ZeroVector;

        // 6. 실제 발사 함수 호출!
        Projectile->FireInDirection(LaunchDirection, LaunchPower, WindForce);

        UE_LOG(LogTemp, Warning, TEXT("투사체 발사 성공!"));
    }
}