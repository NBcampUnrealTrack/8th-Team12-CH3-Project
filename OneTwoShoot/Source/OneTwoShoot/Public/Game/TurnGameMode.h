#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "../Public/Game/TankGameTypes.h"
#include "TurnGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnChangedSignature, ETurnState, NewTurnState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveChangedSignature, int32, NewWaveCount);

UCLASS()
class ONETWOSHOOT_API ATurnGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ATurnGameMode();
	
	UFUNCTION(BlueprintCallable, Category = "Turn System")
	void StartWave();
	
	UFUNCTION(BlueprintCallable, Category = "Turn System")
	void EndCurrentTurn();
	
	UFUNCTION(BlueprintCallable, Category = "Turn System")
	void ContinueEnemyGroupTurn();
	
	UFUNCTION()
	void EndWave();
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTurnChangedSignature OnTurnChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWaveChangedSignature OnWaveChanged;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 CurrentWaveCount;
	
	UFUNCTION(BlueprintPure, Category = "Turn")
	ETurnState GetCurrentTurnState() const { return CurrentTurnState; }
	
	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetCurrentWaveCount() const { return CurrentWaveCount; }

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn System|State")
	ETurnState CurrentTurnState;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn System|State")
	ETankUnitType LastActiveUnit;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn System|Player")
	int32 PlayerTurnCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn System|Player")
	EReloadSpeed CurrentPlayerReloadSpeed;
	
	UPROPERTY()
	TArray<ABaseEnemyTank*> AliveEnemies;
	
	int32 CurrentEnemyIndex;
	
	bool bIsWaveRunning = false;

	void DetermineNextTurn();
	void StartEnemyGroupTurn();
};
