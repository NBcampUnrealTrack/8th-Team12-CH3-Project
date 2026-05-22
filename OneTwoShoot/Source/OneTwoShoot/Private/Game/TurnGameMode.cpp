#include "../Public/Game/TurnGameMode.h"
#include "../Public/Tank/Enemy/BaseEnemyTank.h"
#include "../Public/Tank/Player/PlayerTank.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ATurnGameMode::ATurnGameMode()
{
	CurrentTurnState = ETurnState::Wait;
	LastActiveUnit = ETankUnitType::None;
	PlayerTurnCount = 0;
	CurrentPlayerReloadSpeed = EReloadSpeed::Normal;
}

void ATurnGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ATurnGameMode::StartWave()
{
	bIsWaveRunning = true;

	CurrentTurnState = ETurnState::Wait;
	OnTurnChanged.Broadcast(CurrentTurnState);

	LastActiveUnit = ETankUnitType::Enemy;
	PlayerTurnCount = 0;
	DetermineNextTurn();
}

void ATurnGameMode::EndCurrentTurn()
{
	CurrentTurnState = ETurnState::Wait;
	OnTurnChanged.Broadcast(CurrentTurnState);

	FTimerHandle TurnDelayHandle;
	GetWorldTimerManager().SetTimer(
		TurnDelayHandle, this, &ATurnGameMode::DetermineNextTurn, 2.0f, false);
}

void ATurnGameMode::DetermineNextTurn()
{
	if (!bIsWaveRunning) return;

	if (LastActiveUnit == ETankUnitType::Player)
	{
		PlayerTurnCount++;

		bool bGivePlayerBonusTurn = false;

		// 장전 속도에 따른 보너스 턴 체크 (데이터 추적 유지)
		switch (CurrentPlayerReloadSpeed)
		{
		case EReloadSpeed::Fast:
			if (PlayerTurnCount % 5 == 0) bGivePlayerBonusTurn = true;
			break;
		case EReloadSpeed::VeryFast:
			if (PlayerTurnCount % 3 == 0) bGivePlayerBonusTurn = true;
			break;
		default:
			break;
		}

		if (bGivePlayerBonusTurn)
		{
			UE_LOG(LogTemp, Warning, TEXT("보너스 차례 획득! 플레이어가 연속 행동합니다. (자동 스킵 가동)"));
			CurrentTurnState = ETurnState::PlayerTurn;
			
			OnTurnChanged.Broadcast(CurrentTurnState);
			
			LastActiveUnit = ETankUnitType::Player;

			FTimerHandle SkipTimer;
			GetWorldTimerManager().SetTimer(SkipTimer, this, &ATurnGameMode::EndCurrentTurn, 2.0f, false);
			return;
		}

		CurrentTurnState = ETurnState::EnemyTurn;
		
		OnTurnChanged.Broadcast(CurrentTurnState);
		
		LastActiveUnit = ETankUnitType::Enemy;
		StartEnemyGroupTurn();
	}
	else
	{
		// 장전 속도가 느릴 경우 패널티 체크 (플레이어 차례 무효화)
		if (CurrentPlayerReloadSpeed == EReloadSpeed::Slow && (PlayerTurnCount + 1) % 5 == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("장전 패널티: 이번 플레이어 차례는 무효화됩니다."));
			PlayerTurnCount++;

			CurrentTurnState = ETurnState::EnemyTurn;
			
			OnTurnChanged.Broadcast(CurrentTurnState);
			
			LastActiveUnit = ETankUnitType::Enemy;
			StartEnemyGroupTurn();
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("==== 플레이어 차례 시작 (입력 대기) ===="));
		CurrentTurnState = ETurnState::PlayerTurn;
		
		OnTurnChanged.Broadcast(CurrentTurnState);
		
		LastActiveUnit = ETankUnitType::Player;

		ABaseTank* PlayerTank = Cast<ABaseTank>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
		if (PlayerTank)
		{
			PlayerTank->OnTurnStart();
		}
	}
}
void ATurnGameMode::StartEnemyGroupTurn()
{
	UE_LOG(LogTemp, Warning, TEXT("적 차례 시작"));
	
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseEnemyTank::StaticClass(), FoundEnemies);

	AliveEnemies.Empty();
	for (AActor* EnemyActor : FoundEnemies)
	{
		ABaseEnemyTank* Enemy = Cast<ABaseEnemyTank>(EnemyActor);

		if (Enemy && !Enemy->IsDead())
		{
			AliveEnemies.Add(Enemy);
		}
	}

	if (AliveEnemies.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("살아있는 적이 없습니다. 플레이어 턴으로 전환합니다."));
		EndCurrentTurn();
		return;
	}

	CurrentEnemyIndex = 0;
	ContinueEnemyGroupTurn();
}

void ATurnGameMode::ContinueEnemyGroupTurn()
{
	// 모든 적이 행동을 완료했는지 체크
	if (CurrentEnemyIndex >= AliveEnemies.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 적의 행동 완료. 플레이어 턴으로 전환합니다."));
		EndCurrentTurn();
		return;
	}

	ABaseEnemyTank* CurrentEnemy = Cast<ABaseEnemyTank>(AliveEnemies[CurrentEnemyIndex]);

	if (IsValid(CurrentEnemy) && !CurrentEnemy->IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%d/%d] %s 행동 시작"),
			CurrentEnemyIndex + 1, AliveEnemies.Num(), *CurrentEnemy->GetName());

		// 인덱스를 먼저 올리고 행동을 시작합니다.
		CurrentEnemyIndex++;
		CurrentEnemy->OnTurnStart();
	}
	else
	{
		// 유효하지 않은 적이면 즉시 다음으로
		CurrentEnemyIndex++;
		ContinueEnemyGroupTurn();
	}
}

void ATurnGameMode::EndWave()
{
	bIsWaveRunning = false;
	CurrentTurnState = ETurnState::Wait;
	LastActiveUnit = ETankUnitType::None;

	// 진행 중인 턴 타이머 정리
	GetWorldTimerManager().ClearAllTimersForObject(this);

	UE_LOG(LogTemp, Warning, TEXT("전투 종료. 턴 시스템 대기 상태로 전환합니다."));
}