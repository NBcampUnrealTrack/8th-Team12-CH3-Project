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

    virtual void Fire() override;

    virtual void Aim() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* BarrelPivotComp;

    UPROPERTY(EditAnywhere, Category = "Components")
    USceneComponent* FirePivotComp;

    UPROPERTY(EditAnywhere, Category = "VFX")
    class UNiagaraSystem* MuzzleFlashEffect;

    UPROPERTY(EditAnywhere, Category = "Sound")
    class USoundBase* FireSound;

private:

    void FireProjectile(FRotator SpawnRotation);

    void Pattern_SingleShot();
    void Pattern_SpreadShot(int32 ProjectileCount, float SpreadAngle);

    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<class ABaseProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float LaunchPower;
};