#include "TurnGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
// #include "EnemyPlayer?" -> 적 AI 관련 클래스 헤더 포함

ATurnGameMode::ATurnGameMode()
{
	CurrentTurnState = ETurnState::Wait;
	LastActiveUnit = EUnitType::None;
	PlayerTurnCount = 0;
	CurrentPlayerReloadSpeed = EReloadSpeed::Normal;
}

void ATurnGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ATurnGameMode::StartWave()
{
	UE_LOG(LogTemp, Warning, TEXT("웨이브가 시작됐습니다."));
	
	CurrentTurnState = ETurnState::Wait;
	LastActiveUnit = EUnitType::Enemy;
	PlayerTurnCount = 0;
	
	DetermineNextTurn();
}

void ATurnGameMode::EndCurrentTurn()
{
	CurrentTurnState = ETurnState::Wait;
	
	FTimerHandle TurnDelayHandle;
	GetWorldTimerManager().SetTimer(TurnDelayHandle, this, &ATurnGameMode::DetermineNextTurn, 2.0f, false);
}

void ATurnGameMode::DetermineNextTurn()
{
	if (LastActiveUnit == EUnitType::Player)
	{
		PlayerTurnCount++;
		
		bool bGivePlayerBonusTurn = false;
		
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
			UE_LOG(LogTemp, Warning, TEXT("보너스 차례를 획득합니다. 플레이어가 연속 행동합니다."));
			CurrentTurnState = ETurnState::PlayerTurn;
			LastActiveUnit = EUnitType::None;
			return;
		}
		
		CurrentTurnState = ETurnState::EnemyTurn;
		LastActiveUnit = EUnitType::Enemy;
		StartEnemyGroupTurn();
	} 
	else
	{
		if (CurrentPlayerReloadSpeed == EReloadSpeed::Slow && (PlayerTurnCount + 1) % 5 == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("장전 속도가 느려 패널티로 이번 차례가 무효화됩니다."));
			PlayerTurnCount++;
			
			CurrentTurnState = ETurnState::EnemyTurn;
			LastActiveUnit = EUnitType::Enemy;
			
			StartEnemyGroupTurn();
			return;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("플레이어 차례 시작"));
		CurrentTurnState = ETurnState::PlayerTurn;
		LastActiveUnit = EUnitType::Player;
	}
}

void ATurnGameMode::StartEnemyGroupTurn()
{
	UE_LOG(LogTemp, Warning, TEXT("적 차례 시작"));
	
	AliveEnemies.Empty();
	
	if (AliveEnemies.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("살아있는 적이 없습니다. 웨이브가 종료되거나 다음 차례를 진행합니다."))
		EndCurrentTurn();
		return;
	}
	
	CurrentEnemyIndex = 0;
	ContinueEnemyGroupTurn();
}

void ATurnGameMode::ContinueEnemyGroupTurn()
{
	if (CurrentEnemyIndex < AliveEnemies.Num())
	{
		AActor* CurrentEnemy = AliveEnemies[CurrentEnemyIndex];
		
		if (IsValid(CurrentEnemy))
		{
			UE_LOG(LogTemp, Warning, TEXT("%d 번째 적 행동 시작"), CurrentEnemyIndex);
			
			// 여기서 적의 턴 시작 함수를 호출
			
			// 적이 발사한 투사체가 플레이어에게 명중하고 모든 이펙트가 끝났을 때,
			// 적 AI 코드 내부에서 반드시 아래 코드를 호출하여 게임 모드에 턴이 끝났음을 알려야 함
			
			// ATurnGameMode* GameMode = Cast<ATurnGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			// if (GameMode) GameMode->ContinueEnemyGroupTurn();
		}
		else
		{
			CurrentEnemyIndex++;
			ContinueEnemyGroupTurn();
			return;
		}
		
		CurrentEnemyIndex++;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("적 턴 종료"));
		EndCurrentTurn();
	}
}
