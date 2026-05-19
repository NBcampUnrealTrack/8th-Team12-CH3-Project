#include "../Public/UI/TankUIWidget.h"
#include "../Public/Tank/BaseTank.h"

void UTankUIWidget::BindToTank(ABaseTank* InTank)
{
	if (InTank)
	{
		InTank->OnHealthChanged.AddDynamic(this, &UTankUIWidget::OnTankHealthChanged);
		
		UpdateHealthBar(InTank->GetCurrentHealth(), InTank->GetMaxHealth());
	}
}

void UTankUIWidget::OnTankHealthChanged(float CurrentHealth, float MaxHealth)
{
	UpdateHealthBar(CurrentHealth, MaxHealth);
}