// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyAIMobile.h"
#include "EnemyAIMobileBossBase.generated.h"

/**
 * 
 */
UCLASS()
class ONETWOSHOOT_API AEnemyAIMobileBossBase : public AEnemyAIMobile
{
    GENERATED_BODY()

public:
    AEnemyAIMobileBossBase();

protected:
    virtual void BeginPlay() override;

    // 페이즈 관리
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
    int32 CurrentPhase;

    // 체력 비율 ex) {0.7f, 0.4f} → 70%, 40%에서 페이즈 전환
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss")
    TArray<float> PhaseThresholds;

    // 패턴 관리
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
    bool bIsExecutingPattern; // 패턴 실행 중 여부

    // 보스 체력바 위젯
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI")
    TSubclassOf<UUserWidget> BossHealthBarClass;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|UI")
    UUserWidget* BossHealthBarWidget;

    // 경고 UI
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI")
    TSubclassOf<UUserWidget> WarningUIClass;

    // 오버라이드
    virtual void OnTurnStart() override;
    virtual void OnDamaged(int32 DamageAmount) override;
    virtual void OnDeath() override;
    virtual void DecideAction() override;

    // 보스 전용 함수
    virtual void CheckPhaseTransition();       // 페이즈 전환 체크
    virtual void OnPhaseChanged(int32 NewPhase); // 페이즈 변경 시 호출
    virtual void ExecutePattern();             // 현재 페이즈 패턴 실행 (하위 클래스에서 구현)
    virtual void ShowWarningUI(float Duration); // 특수 공격 전 경고 표시
    void ShowBossHealthBar();                  // 보스 체력바 표시
};