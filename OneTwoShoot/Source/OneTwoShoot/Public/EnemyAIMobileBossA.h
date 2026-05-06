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

    // 베이스 클래스의 패턴 실행 함수 오버라이드
    virtual void ExecutePattern() override;

private:
    // 보스 A만의 전투 관련 기능
    void Aim();
    void FireProjectile(FRotator SpawnRotation);

    // 페이즈별 패턴
    void Pattern_SingleShot();
    void Pattern_SpreadShot(int32 ProjectileCount, float SpreadAngle);

    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<class ABaseProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, Category = "Combat")
    USceneComponent* FirePoint;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ShootPower;
};