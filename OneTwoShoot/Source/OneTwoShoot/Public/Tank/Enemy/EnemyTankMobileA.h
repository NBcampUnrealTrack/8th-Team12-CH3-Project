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

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    TSubclassOf<AActor> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    USceneComponent* FirePoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float CurrentAimAngle; 

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float LaunchPower;

    virtual void DecideAction() override;

    void Aim();

    void Fire();
};