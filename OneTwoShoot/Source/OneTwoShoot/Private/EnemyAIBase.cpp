#include "EnemyAIBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TurnGameMode.h"

AEnemyAIBase::AEnemyAIBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // 기본 스탯 초기값 (블루프린트에서 덮어쓸 수 있음)
    MaxHealth = 100;
    CurrentHealth = MaxHealth;
    AttackDamage = 10;
    MoveSpeed = 300.f;
    bIsDead = false;
    MaxTurnActionCount = 3;
    TurnActionCount = MaxTurnActionCount;

    // CharacterMovement 기본 설정
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
}

// 스폰 시, 체력 초기화 및 플레이어 감지
void AEnemyAIBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    PlayerCheck();
}

void AEnemyAIBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// 피격 데미지 처리
float AEnemyAIBase::TakeDamage(
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

// 피격 로직
void AEnemyAIBase::OnDamaged(int32 DamageAmount)
{
    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0, MaxHealth);

    // 피격 로직 (애니메이션 등)은 하위 클래스에서 추가

    if (CurrentHealth <= 0)
    {
        OnDeath();
    }
}

//사망 로직
void AEnemyAIBase::OnDeath()
{
    if (bIsDead) return;
    bIsDead = true;

    DropItem();

    // 사망 애니메이션, 충돌 비활성화 등은 하위 클래스에서 추가

    // 일정 시간 후 Actor 제거
    SetLifeSpan(3.f);
}

void AEnemyAIBase::DropItem()
{
    // 하위 클래스에서 override하여 랜덤 아이템 스폰 구현
}

// 월드에서 플레이어 캐릭터 바로 가져옴
void AEnemyAIBase::PlayerCheck()
{
    TargetPlayer = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

// 플레이어가 사거리 내인지 체크
bool AEnemyAIBase::IsInAttackRange()
{
    if (!TargetPlayer) return false;

    // 1. 2D 거리 계산 (Z축 무시)
    float Distance = FVector::Dist2D(GetActorLocation(), TargetPlayer->GetActorLocation());

    // 2. 2D 비주얼 디버깅 (바닥에 원 그리기)
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

void AEnemyAIBase::OnTurnStart()
{
    if (bIsDead) return; // 사망 시 턴 무시
    UE_LOG(LogTemp, Warning, TEXT("[%s] OnTurnStart 호출됨"), *GetName());
    // 하위 클래스에서 오버라이드하여 각자 행동 구현
}

void AEnemyAIBase::OnTurnEnd()
{
    TurnActionCount = MaxTurnActionCount;
    // 턴 종료 시 공통 처리
    // ex) 턴 매니저에 종료 알림
    
    // 3. 게임 모드에 자신의 행동이 끝났음을 알림 (다음 적 차례 진행)
    ATurnGameMode* GameMode = Cast<ATurnGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GameMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 행동 종료 - 다음 적 차례 요청"), *GetName());
        GameMode->ContinueEnemyGroupTurn();
    }
}

// 외부에서 현재 체력 가져오기
int32 AEnemyAIBase::GetCurrentHealth() const
{
    return CurrentHealth;
}

// 사망 체크
bool AEnemyAIBase::IsDead() const
{
    return bIsDead;
}