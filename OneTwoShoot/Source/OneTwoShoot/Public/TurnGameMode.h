#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TurnGameMode.generated.h"

UENUM(BlueprintType)
enum class EReloadSpeed : uint8
{
	Slow		UMETA(DisplayName = "느림"),
	Normal		UMETA(DisplayName = "보통"),
	Fast		UMETA(DisplayName = "빠름"),
	VeryFast	UMETA(DisplayName = "매우 빠름")
};

UENUM(BlueprinType)
enum class ETurnState : uint8
{
	Wait		UMETA(DisplayName = "대기 및 처리 중"),
	PlayerTurn	UMETA(DisplayName = "플레이어 차례"),
	EnemyTurn	UMETA(DisplayName = "적 차례")
};

UENUM(BlueprintType)
enum class EUnitType : uint8
{
	None,
	Player,
	Enemy
};

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
