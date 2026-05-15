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

    virtual void OnTurnStart() override;
    virtual void DecideAction();
    virtual void Fire();

    virtual void Landed(const FHitResult& Hit) override;

};

