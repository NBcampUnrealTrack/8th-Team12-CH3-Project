#pragma once

#include "CoreMinimal.h"
#include "Tank/Enemy/BaseEnemyTankMobile.h" // 부모 클래스 경로 확인 필요
#include "EnemyTankSelfDestruct.generated.h"

UCLASS()
class ONETWOSHOOT_API AEnemyTankSelfDestruct : public ABaseEnemyTankMobile
{
    GENERATED_BODY()

public:
    AEnemyTankSelfDestruct();

protected:
    // 상속받은 행동 결정 로직 오버라이드
    virtual void DecideAction() override;

    // 자폭 실행 함수
    void Explode();

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ExplosionDamage;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ExplosionRadius;
};