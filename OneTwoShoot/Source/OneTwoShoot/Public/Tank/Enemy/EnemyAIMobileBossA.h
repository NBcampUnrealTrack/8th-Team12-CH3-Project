#pragma once

#include "CoreMinimal.h"
#include "EnemyAIMobileBossBase.h"
#include "EnemyAIMobileBossA.generated.h"

UCLASS()
class ONETWOSHOOT_API AEnemyAIMobileBossA : public AEnemyAIMobileBossBase
{
    GENERATED_BODY()

public:
    AEnemyAIMobileBossA();

protected:
    virtual void BeginPlay() override;

    virtual void ExecutePattern() override;

private:

    void Aim();
    void FireProjectile(FRotator SpawnRotation);

    void Pattern_SingleShot();
    void Pattern_SpreadShot(int32 ProjectileCount, float SpreadAngle);

    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<class ABaseProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, Category = "Combat")
    USceneComponent* FirePoint;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ShootPower;
};