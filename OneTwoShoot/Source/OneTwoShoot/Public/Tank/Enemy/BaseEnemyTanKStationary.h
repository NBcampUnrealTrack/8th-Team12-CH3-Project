// 움직이지 않는 적 AI Base 클래스

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemyTank.h"
#include "BaseEnemyTanKStationary.generated.h"

UCLASS()
class ONETWOSHOOT_API ABaseEnemyTanKStationary : public ABaseEnemyTank
{
    GENERATED_BODY()

public:
    ABaseEnemyTanKStationary();

protected:
    virtual void BeginPlay() override;

    // 현재 포신 각도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float CurrentAimAngle;

    virtual void Aim();
    virtual void Fire();

    virtual void OnTurnStart() override; // override 키워드 추가
};

