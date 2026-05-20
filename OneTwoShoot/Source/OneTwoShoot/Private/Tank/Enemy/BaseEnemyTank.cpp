#include "../Public/Tank/Enemy/BaseEnemyTank.h"
#include "../Public/World/VoxelWorld.h"
#include "../Public/World/VoxelPathfinder.h"
#include "../Public/Game/TurnGameMode.h"
#include "../Public/Game/StageWaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

ABaseEnemyTank::ABaseEnemyTank()
{
	PrimaryActorTick.bCanEverTick = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyHPBar"));
	OverheadWidget->SetupAttachment(GetMesh());
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);

	MaxHealth = 100;
	CurrentHealth = MaxHealth;
	AttackDamage = 10;
	MoveSpeed = 300.f;
	bIsDead = false;
	MaxTurnActionCount = 3;
	TurnActionCount = MaxTurnActionCount;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	}

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	CurrentAimAngle = 0.f;
}

void ABaseEnemyTank::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	PlayerCheck();

	UpdateOverheadHP();
}

void ABaseEnemyTank::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float ABaseEnemyTank::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	if (bIsDead) return 0.f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	OnDamaged(FMath::RoundToInt(ActualDamage));

	return ActualDamage;
}

void ABaseEnemyTank::OnDamaged(int32 DamageAmount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0, MaxHealth);

	// 피격 로직 (애니메이션 등)은 하위 클래스에서 추가

	UpdateOverheadHP();

	if (CurrentHealth <= 0)
	{
		OnDeath();
	}
}

void ABaseEnemyTank::OnDeath()
{
	if (bIsDead) return;
	bIsDead = true;

	DropItem();

	AActor* FoundManager = UGameplayStatics::GetActorOfClass(GetWorld(), AStageWaveManager::StaticClass());
	if (AStageWaveManager* WaveManager = Cast<AStageWaveManager>(FoundManager))
	{
		WaveManager->OnEnemyDestroyed();
	}


	// 사망 애니메이션, 충돌 비활성화 등은 하위 클래스에서 추가

	SetLifeSpan(3.f);
}

void ABaseEnemyTank::DropItem()
{
	// 하위 클래스에서 override하여 랜덤 아이템 스폰 구현
}

void ABaseEnemyTank::PlayerCheck()
{
	TargetPlayer = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

bool ABaseEnemyTank::IsInAttackRange()
{
	if (!TargetPlayer) return false;

	float Distance = FVector::Dist2D(GetActorLocation(), TargetPlayer->GetActorLocation());

	// 2D 비주얼 디버깅 (바닥에 원 그리기)
	FVector Center = GetActorLocation();
	// 캐릭터의 발바닥 위치로 높이 조정 (필요 시)
	//Center.Z -= 45.f; 

	DrawDebugCircle(
		GetWorld(),
		Center,                // 중심점
		AttackRange,           // 반지름
		32,                    // 세그먼트 (원이 얼마나 부드러운지)
		FColor::Green,         // 색상
		false,                 // 지속성 여부
		0.1f,                  // 수명
		0,                     // 우선순위
		2.0f,                  // 선 두께
		FVector(0, 0, 1)       // 원의 축 (Z축 방향을 바라보게 하여 평면으로 생성)
	);

	UE_LOG(LogTemp, Warning, TEXT("[%s] 2D 거리 체크 - 실제 거리: %f, 사거리: %f"),
		*GetName(), Distance, AttackRange);

	return Distance <= AttackRange;
}

void ABaseEnemyTank::OnTurnStart()
{
	if (bIsDead) return;
	UE_LOG(LogTemp, Warning, TEXT("[%s] OnTurnStart 호출됨"), *GetName());
	DecideAction();
	// 하위 클래스에서 오버라이드하여 각자 행동 구현
}

void ABaseEnemyTank::OnTurnEnd()
{
	TurnActionCount = MaxTurnActionCount;
	// 턴 종료 시 공통 처리
	// ex) 턴 매니저에 종료 알림

	ATurnGameMode* GameMode = Cast<ATurnGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 행동 종료 - 다음 적 차례 요청"), *GetName());
		GameMode->ContinueEnemyGroupTurn();
	}
}

int32 ABaseEnemyTank::GetCurrentHealth() const
{
	return CurrentHealth;
}

bool ABaseEnemyTank::IsDead() const
{
	return bIsDead;
}

void ABaseEnemyTank::UpdateOverheadHP()
{
	if (!OverheadWidget) return;

	UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject();
	if (!OverheadWidgetInstance) return;

	if (UTextBlock* HPText = Cast<UTextBlock>(OverheadWidgetInstance->GetWidgetFromName(TEXT("OverHeadHP"))))
	{
		HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth)));
	}

	TargetPercent = (float)CurrentHealth / (float)MaxHealth;

	if (!GetWorldTimerManager().IsTimerActive(HPBarTimerHandle))
	{
		GetWorldTimerManager().SetTimer(HPBarTimerHandle, this, &ABaseEnemyTank::HandleHPBarLerp, 0.01f, true);
	}
}

void ABaseEnemyTank::HandleHPBarLerp()
{
	CurrentPercent = FMath::FInterpTo(CurrentPercent, TargetPercent, 0.01f, LerpSpeed);

	UUserWidget* WidgetInst = OverheadWidget->GetUserWidgetObject();
	if (WidgetInst)
	{
		if (UProgressBar* HPBar = Cast<UProgressBar>(WidgetInst->GetWidgetFromName(TEXT("HPBar"))))
		{
			HPBar->SetPercent(CurrentPercent);
		}
	}

	if (FMath::IsNearlyEqual(CurrentPercent, TargetPercent, 0.001f))
	{
		CurrentPercent = TargetPercent;
		GetWorldTimerManager().ClearTimer(HPBarTimerHandle);
	}
}

void ABaseEnemyTank::Aim()
{
	if (!TargetPlayer) return;

	FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
	FRotator LookAt = Direction.Rotation();
	SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
	CurrentAimAngle = LookAt.Pitch;

	UE_LOG(LogTemp, Warning, TEXT("[%s] 조준 완료 - Yaw: %f, Pitch: %f"),
		*GetName(), LookAt.Yaw, LookAt.Pitch);
}

void ABaseEnemyTank::DecideAction()
{
	if (bIsDead) return;

	UE_LOG(LogTemp, Warning, TEXT("========================================"));
	UE_LOG(LogTemp, Warning, TEXT("[판단 뇌] %s 의 DecideAction 가동! (남은 행동력: %d)"), *GetName(), TurnActionCount);

	if (IsInAttackRange())
	{
		UE_LOG(LogTemp, Warning, TEXT("  -> [결정] 사거리 내 진입 확인. 조준 및 사격(Fire) 실시."));
		Aim();
		Fire();

		FTimerHandle ActionDelayHandle;
		GetWorldTimerManager().SetTimer(ActionDelayHandle, this, &ABaseEnemyTank::OnTurnEnd, 1.5f, false);
	}
	else if (TurnActionCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("  -> [결정] 사거리 밖. 타겟을 향해 주행(MoveOnVoxelGrid) 요청."));
		MoveOnVoxelGrid();
		--TurnActionCount;
		UE_LOG(LogTemp, Warning, TEXT("  -> [결정] 행동력 차감 완료 (현재 남은 행동력: %d)"), TurnActionCount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("  -> [결정] 행동력 모두 소진. 턴 종료(OnTurnEnd) 요청."));
		OnTurnEnd();
	}
	UE_LOG(LogTemp, Warning, TEXT("========================================"));
}

void ABaseEnemyTank::MoveOnVoxelGrid()
{
}

void ABaseEnemyTank::Fire()
{
}