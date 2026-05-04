// 잡몹 A(움직임 + 포탄 사격)

#pragma once

#include "CoreMinimal.h"
#include "EnemyAIMobile.h"
#include "EnemyAIMobileA.generated.h"

UCLASS()
class ONETWOSHOOT_API AEnemyAIMobileA : public AEnemyAIMobile
{
	GENERATED_BODY()

public:
    AEnemyAIMobileA();

protected:
    virtual void BeginPlay() override;

    // 발사할 투사체 클래스
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    TSubclassOf<AActor> ProjectileClass;

    // 발사 위치
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    USceneComponent* FirePoint;

    // 현재 조준 각도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float CurrentAimAngle; 

    // 이동 후 행동 결정 오버라이드
    virtual void DecideAction() override;

    // 플레이어 방향 조준
    void Aim();

    // 투사체 발사
    void Fire();
};