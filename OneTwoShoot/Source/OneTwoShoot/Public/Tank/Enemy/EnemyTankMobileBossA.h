#pragma once

#include "CoreMinimal.h"
#include "BaseEnemyTankMobileBoss.h"
#include "EnemyTankMobileBossA.generated.h"

UCLASS()
class ONETWOSHOOT_API AEnemyTankMobileBossA : public ABaseEnemyTankMobileBoss
{
    GENERATED_BODY()

public:
    AEnemyTankMobileBossA();

protected:
    virtual void BeginPlay() override;

    virtual void ExecutePattern() override;

private:

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