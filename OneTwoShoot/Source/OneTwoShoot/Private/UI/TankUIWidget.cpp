#include "../Public/UI/TankUIWidget.h"
#include "../Public/Tank/BaseTank.h"
#include "../Public/Game/TurnGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Tank/Player/PlayerTank.h"

void UTankUIWidget::BindToTank(ABaseTank* InTank)
{
	if (InTank)
	{
		InTank->OnHealthChanged.AddDynamic(this, &UTankUIWidget::OnTankHealthChanged);
		
		UpdateHealthBar(InTank->GetCurrentHealth(), InTank->GetMaxHealth());
		
		if (APlayerTank* PlayerTank = Cast<APlayerTank>(InTank))
		{
			UE_LOG(LogTemp, Warning, TEXT("==== 성공: UI가 플레이어 탱크와 정상적으로 연결되었습니다! ===="));
			
			PlayerTank->OnPlayerPhaseChanged.AddDynamic(this, &UTankUIWidget::OnTankPhaseChanged);
			
			UpdateControlGuideUI(PlayerTank->GetCurrentPhase());
		} else
		{
			UE_LOG(LogTemp, Error, TEXT("==== 실패: 들어온 탱크가 플레이어 탱크가 아닙니다! ===="));
		}
	}
}

void UTankUIWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ATurnGameMode* GameMode = Cast<ATurnGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	
	if (GameMode)
	{
		GameMode->OnTurnChanged.AddDynamic(this, &UTankUIWidget::OnGameTurnChanged);
		UpdateTurnDisplay(GameMode->GetCurrentTurnState());
		
		GameMode->OnWaveChanged.AddDynamic(this, &UTankUIWidget::OnGameWaveChanged);
		UpdateWaveDisplay(GameMode->GetCurrentWaveCount());
	}
}

void UTankUIWidget::OnTankHealthChanged(float CurrentHealth, float MaxHealth)
{
	UpdateHealthBar(CurrentHealth, MaxHealth);
}

void UTankUIWidget::OnGameTurnChanged(ETurnState NewTurnState)
{
	UpdateTurnDisplay(NewTurnState);
}

void UTankUIWidget::OnGameWaveChanged(int32 NewWaveCount)
{
	UpdateWaveDisplay(NewWaveCount);
}

void UTankUIWidget::OnTankPhaseChanged(ETankPhase NewPhase)
{
	UpdateControlGuideUI(NewPhase);
}
