#include "../Public/Tank/BaseTank.h"
#include "../Public/Tank/BaseProjectile.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "Kismet/GameplayStatics.h"

ABaseTank::ABaseTank()
{
 	PrimaryActorTick.bCanEverTick = true;
	CurrentPhase = ETankPhase::Wait;
}

void ABaseTank::SetTankPhase(ETankPhase NewPhase)
{
	CurrentPhase = NewPhase;
}

void ABaseTank::UpdateAimAngle(float PitchDelta, float YawDelta)
{
	if (CurrentPhase == ETankPhase::Aim)
	{
		CurrentPitch = FMath::Clamp(CurrentPitch + PitchDelta, 0.0f, 90.0f);
		CurrentYaw += YawDelta;
		
		// 이후 포신 (메시 형태) 를 회전시키는 시각적 처리 추가
	}
}

void ABaseTank::FireCannon()
{
	if (CurrentPhase != ETankPhase::Action && CurrentPhase != ETankPhase::Aim) return;
	
	if (ProjectileClass && GetWorld())
	{
		FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
		FRotator ShootRotation = FRotator(CurrentPitch, CurrentYaw, 0.0f);
		FVector ShootDirection = ShootRotation.Vector();
		
		float FinalPower = TankBasePower;
		
		FTransform SpawnTransform(ShootRotation, SpawnLocation);
		ABaseProjectile* SpawnedProjectile = GetWorld()->SpawnActorDeferred<ABaseProjectile>(
			ProjectileClass,
			SpawnTransform,
			this,
			GetInstigator(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
			);
		
		if (SpawnedProjectile)
		{
			// 바람 데이터는 게임 모드에서 가져오도록 처리 필요 (현재 임시로 빈 벡터 전달)
			
			SpawnedProjectile->FireInDirection(ShootDirection, FinalPower, FVector::ZeroVector);
			UGameplayStatics::FinishSpawningActor(SpawnedProjectile, SpawnTransform);
		}
		
		// 발사 후 턴 종료 처리를 위해 게임 모드에 알리는 로직 추가 필요
	}
}

float ABaseTank::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);
	
	if (CurrentHealth <= 0.0f)
	{
		// 사망 처리 (파괴 이펙트나 게임 모드에 사망 알림을 보내기 등)
	}
	
	return ActualDamage;
}
