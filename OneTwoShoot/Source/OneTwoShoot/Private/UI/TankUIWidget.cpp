#include "../Public/UI/TankUIWidget.h"
#include "../Public/Tank/BaseTank.h"
#include "../Public/Game/TurnGameMode.h"
#include "Kismet/GameplayStatics.h"

void UTankUIWidget::BindToTank(ABaseTank* InTank)
{
	if (InTank)
	{
		InTank->OnHealthChanged.AddDynamic(this, &UTankUIWidget::OnTankHealthChanged);
		
		UpdateHealthBar(InTank->GetCurrentHealth(), InTank->GetMaxHealth());
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
