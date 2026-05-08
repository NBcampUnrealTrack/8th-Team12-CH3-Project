//전체 적 AI Base 클래스

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemyTank.generated.h"

UCLASS()
class ONETWOSHOOT_API ABaseEnemyTank : public ACharacter
{
    GENERATED_BODY()

public:
    ABaseEnemyTank();

protected:
    virtual void BeginPlay() override;

    // 최대 체력
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI Status")
    int32 MaxHealth;

    // 현재 체력
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Status")
    int32 CurrentHealth;

    // 이동 속도
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI Status")
    float MoveSpeed;

    // 사망 여부 플래그 (중복 사망 처리 방지)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Status")
    bool bIsDead;

    // 탐지된 플레이어 저장
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
    ACharacter* TargetPlayer;

    // 데미지
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    int32 AttackDamage;

    // 공격 사거리
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    //턴당 행동력 최대치
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    int32 MaxTurnActionCount;

    //현재 턴당 행동력
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    int32 TurnActionCount;

    // 순수 가상함수 - 하위 클래스에서 반드시 구현
    virtual void OnDamaged(int32 DamageAmount);
    virtual void OnDeath();
    virtual void DropItem();

    void PlayerCheck();

    // 사거리 내인지 확인
    bool IsInAttackRange();

    // 턴 종료 처리
    UFUNCTION()
    virtual void OnTurnEnd();

public:
    virtual void Tick(float DeltaTime) override;

    // 피격 데미지
    virtual float TakeDamage(float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        AController* EventInstigator,
        AActor* DamageCauser) override;

    // 현재 체력 가져오기
    UFUNCTION(BlueprintCallable, Category = "AI Status")
    int32 GetCurrentHealth() const;

    // 사망 체크;
    UFUNCTION(BlueprintCallable, Category = "AI Status")
    bool IsDead() const;

    // 턴 시작 처리(외부 호출)
    UFUNCTION(BlueprintCallable)
    virtual void OnTurnStart();
};