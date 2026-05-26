#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/TankGameTypes.h"
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
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnGameTurnChanged(ETurnState NewTurnState);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateTurnDisplay(ETurnState NewTurnState);
	
	UFUNCTION()
	void OnGameWaveChanged(int32 NewWaveCount);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateWaveDisplay(int32 NewWaveCount);
	
	UFUNCTION()
	void OnTankHealthChanged(float CurrentHealth, float MaxHealth);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateHealthBar(float CurrentHealth, float MaxHealth);
};