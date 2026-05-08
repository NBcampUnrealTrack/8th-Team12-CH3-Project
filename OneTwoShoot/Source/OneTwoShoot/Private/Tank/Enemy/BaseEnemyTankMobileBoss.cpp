#include "../Public/Tank/Enemy/BaseEnemyTankMobileBoss.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ABaseEnemyTankMobileBoss::ABaseEnemyTankMobileBoss()
{
    CurrentPhase = 0;
    bIsExecutingPattern = false;

    PhaseThresholds.Add(0.7f);
    PhaseThresholds.Add(0.4f);

    MaxHealth = 500;
    AttackRange = 1500.f;
    MoveRange = 300.f;
}

void ABaseEnemyTankMobileBoss::BeginPlay()
{
    Super::BeginPlay();

    //체력 바 제작 후 연결
    //ShowBossHealthBar();

    UE_LOG(LogTemp, Warning, TEXT("[%s] 보스 스폰"), *GetName());
}

void ABaseEnemyTankMobileBoss::OnTurnStart()
{
    if (bIsDead) return;

    UE_LOG(LogTemp, Warning, TEXT("[%s] 보스 턴 시작 - 페이즈: %d"), *GetName(), CurrentPhase);

    if (bIsExecutingPattern)
    {
        ExecutePattern();
        return;
    }

    Super::OnTurnStart();
}

void ABaseEnemyTankMobileBoss::DecideAction()
{
    if (IsInAttackRange())
    {
        ExecutePattern();
    }
    else if (TurnActionCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 보스 추적 이동 - 남은 턴 카운트: %d"), *GetName(), TurnActionCount);
        MoveOnVoxelGrid();
        --TurnActionCount;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 보스 이동 횟수 소진 - 턴 종료"), *GetName());
        OnTurnEnd();
    }
}

void ABaseEnemyTankMobileBoss::OnDamaged(int32 DamageAmount)
{
    Super::OnDamaged(DamageAmount);

    CheckPhaseTransition();
}

void ABaseEnemyTankMobileBoss::CheckPhaseTransition()
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

void ABaseEnemyTankMobileBoss::OnPhaseChanged(int32 NewPhase)
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] 페이즈 %d 전환!"), *GetName(), NewPhase);

    // 페이즈 전환 시 경고 UI 표시 추가시에 사용
    //ShowWarningUI(2.f);

    // 하위 클래스에서 페이즈별 행동 추가
}

void ABaseEnemyTankMobileBoss::ExecutePattern()
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] 기본 패턴 실행 (하위 클래스에서 구현 필요)"), *GetName());
}

// UI 제작 후 연결.
void ABaseEnemyTankMobileBoss::ShowWarningUI(float Duration)
{
    if (!WarningUIClass) return;

    UUserWidget* Warning = CreateWidget<UUserWidget>(GetWorld(), WarningUIClass);
    if (Warning)
    {
        Warning->AddToViewport();

        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, [Warning]()
            {
                if (Warning) Warning->RemoveFromParent();
            }, Duration, false);
    }
}

// 체력 바 제작 후 연결
void ABaseEnemyTankMobileBoss::ShowBossHealthBar()
{
    if (!BossHealthBarClass) return;

    BossHealthBarWidget = CreateWidget<UUserWidget>(GetWorld(), BossHealthBarClass);
    if (BossHealthBarWidget)
    {
        BossHealthBarWidget->AddToViewport();
    }
}

void ABaseEnemyTankMobileBoss::OnDeath()
{
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