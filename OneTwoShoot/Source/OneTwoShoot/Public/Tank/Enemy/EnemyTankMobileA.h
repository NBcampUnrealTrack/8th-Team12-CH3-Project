// 잡몹 A(움직임 + 포탄 사격)

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemyTankMobile.h"
#include "EnemyTankMobileA.generated.h"

UCLASS()
class ONETWOSHOOT_API AEnemyTankMobileA : public ABaseEnemyTankMobile
{
	GENERATED_BODY()

public:
    AEnemyTankMobileA();

    UPROPERTY(EditAnywhere, Category = "VFX")
    class UNiagaraSystem* MuzzleFlashEffect;

    UPROPERTY(EditAnywhere, Category = "Sound")
    class USoundBase* FireSound;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    TSubclassOf<AActor> ProjectileClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float LaunchPower;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
    USceneComponent* FirePivotComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* BarrelPivotComp;

    virtual void Aim() override;

    virtual void Fire() override;
};