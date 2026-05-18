#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TankUIWidget.generated.h"

class ABaseTank;

UCLASS()
class ONETWOSHOOT_API UTankUIWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void BindToTank(ABaseTank* InTank);
	
protected:
	UFUNCTION()
	void OnTankHealthChanged(float CurrentHealth, float MaxHealth);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateHealthBar(float CurrentHealth, float MaxHealth);
};
