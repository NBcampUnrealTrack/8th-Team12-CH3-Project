#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "../Public/Game/TankGameTypes.h"
#include "TurnGameMode.generated.h"

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
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn System|State")
	ETurnState CurrentTurnState;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn System|State")
	EUnitType LastActiveUnit;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn System|Player")
	int32 PlayerTurnCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn System|Player")
	EReloadSpeed CurrentPlayerReloadSpeed;
	
	UPROPERTY()
	TArray<AActor*> AliveEnemies;
	
	int32 CurrentEnemyIndex;
	
	void DetermineNextTurn();
	void StartEnemyGroupTurn();
};
