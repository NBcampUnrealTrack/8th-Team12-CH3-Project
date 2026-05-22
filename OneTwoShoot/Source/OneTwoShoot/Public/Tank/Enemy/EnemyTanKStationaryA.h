#pragma once

#include "CoreMinimal.h"
#include "Tank/Enemy/BaseEnemyTanKStationary.h"
#include "EnemyTanKStationaryA.generated.h"

UCLASS()
class ONETWOSHOOT_API AEnemyTanKStationaryA : public ABaseEnemyTanKStationary
{
    GENERATED_BODY()

public:
    AEnemyTanKStationaryA();

protected:
    void BeginPlay() override;

    void OnTurnStart() override;
    void Fire() override;
    void Aim() override;
    void DecideAction() override;

    bool IsChargedTurn();

    UPROPERTY(EditAnywhere, Category = "Combat")
    int32 MaxChargeTurns = 2; // n턴째에 발사 (기본값 2: 1턴 차징, 2턴 발사)

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    int32 CurrentChargeTurns = 0;

    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<class ABaseProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float LaunchPower;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* FirePivotComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* BarrelPivotComp;
};