// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemyTankMobile.h"
#include "BaseEnemyTankMobileBoss.generated.h"

/**
 * 
 */
UCLASS()
class ONETWOSHOOT_API ABaseEnemyTankMobileBoss : public ABaseEnemyTankMobile
{
    GENERATED_BODY()

public:
    ABaseEnemyTankMobileBoss();

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