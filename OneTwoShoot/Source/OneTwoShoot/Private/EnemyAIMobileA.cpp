// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyAIMobileA.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyAIMobileA::AEnemyAIMobileA()
{
    CurrentAimAngle = 0.f;

    // 발사 위치 컴포넌트 생성
    FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
    FirePoint->SetupAttachment(RootComponent);
    AttackRange = 500.f;
}

void AEnemyAIMobileA::BeginPlay()
{
    Super::BeginPlay();
}

void AEnemyAIMobileA::DecideAction()
{
    if (IsInAttackRange())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 사거리 안 - 조준 및 발사"), *GetName());
        Aim();
        // Fire(); // 투사체 없으니 주석 처리
        OnTurnEnd();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 사거리 밖 - 재이동"), *GetName());
        Move();
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

    // 투사체 스폰
    GetWorld()->SpawnActor<AActor>(
        ProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    // 발사 후 턴 종료
    OnTurnEnd();
}