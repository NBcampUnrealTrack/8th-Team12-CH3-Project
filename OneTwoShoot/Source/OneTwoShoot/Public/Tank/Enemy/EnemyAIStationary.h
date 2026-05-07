// 움직이지 않는 적 AI Base 클래스

#pragma once

#include "CoreMinimal.h"
#include "EnemyAIBase.h"
#include "EnemyAIStationary.generated.h"

UCLASS()
class ONETWOSHOOT_API AEnemyAIStationary : public AEnemyAIBase
{
    GENERATED_BODY()

public:
    AEnemyAIStationary();

protected:
    virtual void BeginPlay() override;

    // 현재 포신 각도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float CurrentAimAngle;

    virtual void Aim();
    virtual void Fire();

    virtual void OnTurnStart() override; // override 키워드 추가
};

