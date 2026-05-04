#include "EnemyAIBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyAIBase::AEnemyAIBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // 기본 스탯 초기값 (블루프린트에서 덮어쓸 수 있음)
    MaxHealth = 100;
    CurrentHealth = MaxHealth;
    AttackDamage = 10;
    MoveSpeed = 300.f;
    bIsDead = false;

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

    float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
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
    // 턴 종료 시 공통 처리
    // ex) 턴 매니저에 종료 알림
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