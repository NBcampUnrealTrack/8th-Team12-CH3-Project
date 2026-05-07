#include "TurnGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EnemyAIBase.h"

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

	// 인게임 용, 플레이어 턴이 먼저 와야해서, 마지막 행동 유닛이 적이여야함.
	//LastActiveUnit = EUnitType::Enemy;

	// 테스트 용, 플레이어가 만들어지지 않아서 바로 EnemyAI 턴으로 전환하기 위함
	LastActiveUnit = EUnitType::Player;

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
	
	// 1. 월드 내 모든 적 AI를 찾아 AliveEnemies 리스트에 담기
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyAIBase::StaticClass(), FoundEnemies);

	AliveEnemies.Empty();
	for (AActor* EnemyActor : FoundEnemies)
	{
		AEnemyAIBase* Enemy = Cast<AEnemyAIBase>(EnemyActor);
		// 살아있는 적만 추가
		if (Enemy && !Enemy->IsDead())
		{
			AliveEnemies.Add(Enemy);
		}
	}

	if (AliveEnemies.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("살아있는 적이 없습니다. 턴을 종료합니다."));
		EndCurrentTurn();
		return;
	}
	
	CurrentEnemyIndex = 0;
	ContinueEnemyGroupTurn();
}

void ATurnGameMode::ContinueEnemyGroupTurn()
{
	// 리스트 범위를 벗어났는지 체크 (모든 적 행동 완료)
	if (CurrentEnemyIndex >= AliveEnemies.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 적의 행동이 완료되었습니다."));
		EndCurrentTurn();
		return;
	}

	AEnemyAIBase* CurrentEnemy = Cast<AEnemyAIBase>(AliveEnemies[CurrentEnemyIndex]);

	if (IsValid(CurrentEnemy) && !CurrentEnemy->IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%d/%d] %s 행동 시작"),
			CurrentEnemyIndex + 1, AliveEnemies.Num(), *CurrentEnemy->GetName());

		// 다음 적을 가리키도록 인덱스 미리 증가
		CurrentEnemyIndex++;

		// 2. 적 AI의 턴 시작 함수 호출
		CurrentEnemy->OnTurnStart();
	}
	else
	{
		// 유효하지 않거나 죽은 적이면 다음 인덱스로 건너뜀
		CurrentEnemyIndex++;
		ContinueEnemyGroupTurn();
	}
}
