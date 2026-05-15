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
    void OnTurnStart() override;
    void Fire() override;
    void DecideAction() override;

    bool IsChargedTurn();

    // --- 차징 로직용 변수 ---
    UPROPERTY(EditAnywhere, Category = "Combat")
    int32 MaxChargeTurns = 2; // n턴째에 발사 (기본값 2: 1턴 차징, 2턴 발사)

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    int32 CurrentChargeTurns = 0;

    // --- 투사체 설정 ---
    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<class ABaseProjectile> ProjectileClass;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    USceneComponent* FirePoint;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float FirePower = 3000.f;
};