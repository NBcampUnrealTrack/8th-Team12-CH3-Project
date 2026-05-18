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
	// 이미 웨이브가 진행 중이면 중복 실행 방지
	if (bIsWaveRunning) return;

	UE_LOG(LogTemp, Warning, TEXT("웨이브가 시작됐습니다."));

	bIsWaveRunning = true;

	CurrentTurnState = ETurnState::Wait;

	// 인게임 용, 플레이어 턴이 먼저 와야해서, 마지막 행동 유닛이 적이여야함.
	LastActiveUnit = ETankUnitType::Enemy;

	// 테스트 용, 플레이어가 만들어지지 않아서 바로 EnemyAI 턴으로 전환하기 위함
	//LastActiveUnit = ETankUnitType::Player;

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
	if (!bIsWaveRunning) return;

	// 적의 차례로 전환

	// 적의 차례로 전환
	if (LastActiveUnit == ETankUnitType::Player)
	{
		PlayerTurnCount++; // 플레이어가 행동을 마쳤으므로 카운트 증가

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
			LastActiveUnit = ETankUnitType::Player; // 연속 행동을 위해 다시 Player로 설정

			// 플레이어 로직 미완성이므로 자동 종료 타이머 가동
			FTimerHandle SkipTimer;
			GetWorldTimerManager().SetTimer(SkipTimer, this, &ATurnGameMode::EndCurrentTurn, 2.0f, false);
			return;
		}

		// 일반적인 경우 적군 턴 시작
		CurrentTurnState = ETurnState::EnemyTurn;
		LastActiveUnit = ETankUnitType::Enemy;
		StartEnemyGroupTurn();
	}
	// 플레이어의 차례로 전환
	else
	{
		// 장전 속도가 느릴 경우 패널티 체크 (플레이어 차례 무효화)
		if (CurrentPlayerReloadSpeed == EReloadSpeed::Slow && (PlayerTurnCount + 1) % 5 == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("장전 패널티: 이번 플레이어 차례는 무효화됩니다."));
			PlayerTurnCount++;

			CurrentTurnState = ETurnState::EnemyTurn;
			LastActiveUnit = ETankUnitType::Enemy;
			StartEnemyGroupTurn();
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("==== 플레이어 차례 시작 (입력 대기) ===="));
		CurrentTurnState = ETurnState::PlayerTurn;
		LastActiveUnit = ETankUnitType::Player;

		ABaseTank* PlayerTank = Cast<ABaseTank>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
		if (PlayerTank)
		{
			PlayerTank->OnTurnStart();
		}

		// 플레이어 조작 대신 턴을 자동으로 넘겨주는 타이머
		// 플레이어 조작 구현 완료로 해당 줄은 주석 처리.
		//FTimerHandle SkipTimer;
		//GetWorldTimerManager().SetTimer(SkipTimer, this, &ATurnGameMode::EndCurrentTurn, 2.0f, false);
	}
}
void ATurnGameMode::StartEnemyGroupTurn()
{
	UE_LOG(LogTemp, Warning, TEXT("적 차례 시작"));
	
	// 월드 내 모든 적 AI를 찾아 AliveEnemies 리스트에 담기
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseEnemyTank::StaticClass(), FoundEnemies);

	AliveEnemies.Empty();
	for (AActor* EnemyActor : FoundEnemies)
	{
		ABaseEnemyTank* Enemy = Cast<ABaseEnemyTank>(EnemyActor);
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
	UE_LOG(LogTemp, Warning, TEXT("턴 시스템이 다음 웨이브를 위해 대기 상태로 전환됩니다."));
}