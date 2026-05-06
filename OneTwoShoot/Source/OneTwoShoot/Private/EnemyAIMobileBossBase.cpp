// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIMobileBossBase.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AEnemyAIMobileBossBase::AEnemyAIMobileBossBase()
{
    // 기본값
    CurrentPhase = 0;
    bIsExecutingPattern = false;

    // 기본 페이즈 임계값 (70%, 40%)
    PhaseThresholds.Add(0.7f);
    PhaseThresholds.Add(0.4f);

    // 보스는 체력, 사거리 높게
    MaxHealth = 500;
    AttackRange = 1500.f;
    MoveRange = 300.f;
}

void AEnemyAIMobileBossBase::BeginPlay()
{
    Super::BeginPlay(); // PlayerCheck() 포함

    //체력 바 제작 후 연결
    //ShowBossHealthBar();

    UE_LOG(LogTemp, Warning, TEXT("[%s] 보스 스폰"), *GetName());
}

void AEnemyAIMobileBossBase::OnTurnStart()
{
    if (bIsDead) return;

    UE_LOG(LogTemp, Warning, TEXT("[%s] 보스 턴 시작 - 페이즈: %d"), *GetName(), CurrentPhase);

    // 패턴 실행 중이면 이동 생략하고 바로 패턴
    if (bIsExecutingPattern)
    {
        ExecutePattern();
        return;
    }

    // 기본 Mobile 흐름 유지
    Super::OnTurnStart();
}

void AEnemyAIMobileBossBase::DecideAction()
{
    if (IsInAttackRange())
    {
        // 사거리 내에 진입하면 패턴 실행
        ExecutePattern();
    }
    else if (TurnActionCount > 0)
    {
        // 사거리 밖이고 행동 횟수가 남았으면 이동
        UE_LOG(LogTemp, Warning, TEXT("[%s] 보스 추적 이동 - 남은 턴 카운트: %d"), *GetName(), TurnActionCount);
        Move();
        --TurnActionCount;
    }
    else
    {
        // 이동 횟수를 다 썼는데도 사거리 밖이면 턴 종료
        UE_LOG(LogTemp, Warning, TEXT("[%s] 보스 이동 횟수 소진 - 턴 종료"), *GetName());
        OnTurnEnd();
    }
}

void AEnemyAIMobileBossBase::OnDamaged(int32 DamageAmount)
{
    Super::OnDamaged(DamageAmount); // 체력 감소 처리

    // 페이즈 전환 체크
    CheckPhaseTransition();
}

void AEnemyAIMobileBossBase::CheckPhaseTransition()
{
    float HealthRatio = (float)CurrentHealth / (float)MaxHealth;

    for (int32 i = CurrentPhase; i < PhaseThresholds.Num(); i++)
    {
        if (HealthRatio <= PhaseThresholds[i])
        {
            CurrentPhase = i + 1;
            OnPhaseChanged(CurrentPhase);
            break;
        }
    }
}

void AEnemyAIMobileBossBase::OnPhaseChanged(int32 NewPhase)
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] 페이즈 %d 전환!"), *GetName(), NewPhase);

    // 페이즈 전환 시 경고 UI 표시
    ShowWarningUI(2.f);

    // 하위 클래스에서 페이즈별 행동 추가
}

void AEnemyAIMobileBossBase::ExecutePattern()
{
    // 하위 클래스에서 오버라이드하여 패턴 구현
    UE_LOG(LogTemp, Warning, TEXT("[%s] 기본 패턴 실행 (하위 클래스에서 구현 필요)"), *GetName());
}

// UI 제작 후 연결.
void AEnemyAIMobileBossBase::ShowWarningUI(float Duration)
{
    if (!WarningUIClass) return;

    UUserWidget* Warning = CreateWidget<UUserWidget>(GetWorld(), WarningUIClass);
    if (Warning)
    {
        Warning->AddToViewport();
        // Duration 후 제거
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, [Warning]()
            {
                if (Warning) Warning->RemoveFromParent();
            }, Duration, false);
    }
}

// 체력 바 제작 후 연결
void AEnemyAIMobileBossBase::ShowBossHealthBar()
{
    if (!BossHealthBarClass) return;

    BossHealthBarWidget = CreateWidget<UUserWidget>(GetWorld(), BossHealthBarClass);
    if (BossHealthBarWidget)
    {
        BossHealthBarWidget->AddToViewport();
    }
}

void AEnemyAIMobileBossBase::OnDeath()
{
    // 체력바 제거
    if (BossHealthBarWidget)
    {
        BossHealthBarWidget->RemoveFromParent();
    }

    UE_LOG(LogTemp, Warning, TEXT("[%s] 보스 처치 - 패시브 스킬 지급"), *GetName());

    // 패시브 스킬 지급 이벤트 발행
    // 턴 매니저 또는 게임 매니저에 알림
    // ex) UGameplayStatics::GetGameMode(GetWorld())->OnBossDefeated(this);

    Super::OnDeath();
}