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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
    int32 CurrentPhase;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss")
    TArray<float> PhaseThresholds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
    bool bIsExecutingPattern;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI")
    TSubclassOf<UUserWidget> BossHealthBarClass;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|UI")
    UUserWidget* BossHealthBarWidget;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI")
    TSubclassOf<UUserWidget> WarningUIClass;

    virtual void OnTurnStart() override;
    virtual void OnDamaged(int32 DamageAmount) override;
    virtual void OnDeath() override;
    virtual void DecideAction() override;

    virtual void CheckPhaseTransition();
    virtual void OnPhaseChanged(int32 NewPhase);
    virtual void ExecutePattern();
    virtual void ShowWarningUI(float Duration);
    void ShowBossHealthBar();
};