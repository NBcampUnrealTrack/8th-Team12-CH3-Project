//전체 적 AI Base 클래스

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemyTank.generated.h"

class UWidgetComponent;

UCLASS()
class ONETWOSHOOT_API ABaseEnemyTank : public ACharacter
{
    GENERATED_BODY()

public:
    ABaseEnemyTank();

protected:

    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI Status")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Status")
    float CurrentHealth;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI Status")
    float MoveSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Status")
    bool bIsDead;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
    ACharacter* TargetPlayer;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    int32 AttackDamage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    //턴당 행동력 최대치
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    int32 MaxTurnActionCount;

    //현재 턴당 행동력
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
    int32 TurnActionCount;

    virtual void OnDamaged(int32 DamageAmount);
    virtual void OnDeath();
    virtual void DropItem();

    void PlayerCheck();

    bool IsInAttackRange();

    UFUNCTION()
    virtual void OnTurnEnd();

public:

    //체력 바 UI
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    UWidgetComponent* OverheadWidget;
    
    //체력이 줄어드는 UI 애니메이션 부드러움 정도
    UPROPERTY(EditAnywhere, Category = "UI")
    float LerpSpeed = 5.f;

    float TargetPercent = 1.f;
    float CurrentPercent = 1.f;
    FTimerHandle HPBarTimerHandle;

    virtual void Tick(float DeltaTime) override;

    virtual float TakeDamage(float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        AController* EventInstigator,
        AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category = "AI Status")
    int32 GetCurrentHealth() const;

    UFUNCTION(BlueprintCallable, Category = "AI Status")
    bool IsDead() const;

    //체력 바 UI 업데이트
    void UpdateOverheadHP();

    //체력 바 UI Lerp 기능
    void HandleHPBarLerp();

    // 턴 시작 처리(외부 호출)
    UFUNCTION(BlueprintCallable)
    virtual void OnTurnStart();
};