#include "../Public/Tank/BaseTank.h"
#include "../Public/Tank/BaseProjectile.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "Game/TurnGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "ModuleDescriptor.h"

ABaseTank::ABaseTank()
{
 	PrimaryActorTick.bCanEverTick = true;
	CurrentPhase = ETankPhase::Wait;
}

void ABaseTank::SetTankPhase(ETankPhase NewPhase)
{
	CurrentPhase = NewPhase;
}

void ABaseTank::BeginPlay()
{
	Super::BeginPlay();

	TArray<UStaticMeshComponent*> MeshComps;
	GetComponents<UStaticMeshComponent>(MeshComps);
	for (UStaticMeshComponent* CurrMesh : MeshComps)
	{
		if (CurrMesh)
		{
			if (CurrMesh->GetName() == TEXT("Turret"))
			{
				CachedTurretMesh = CurrMesh;
			}
			else if (CurrMesh->GetName() == TEXT("Barrel"))
			{
				CachedBarrelMesh = CurrMesh;
			}
		}
	}
	TArray<USceneComponent*> SceneComps;
	GetComponents<USceneComponent>(SceneComps);
	for (USceneComponent* CurrComp : SceneComps)
	{
		if (CurrComp && CurrComp->GetName() == TEXT("BarrelPivot"))
		{
			CachedBarrelPivotComp = CurrComp;
			break;
			}
		}

	CurrentYaw = DefaultTurretRotation.Yaw;
	CurrentPitch = -DefaultBarrelRotation.Pitch;
}

void ABaseTank::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
}

/// ----- 탱크 회전 로직 마스터 본체
void ABaseTank::UpdateAimAngle(float PitchDelta, float YawDelta)
{
	if (CurrentPhase != ETankPhase::Aim) return;

	// 터렛 좌우 회전
	if (CachedTurretMesh && !FMath::IsNearlyZero(YawDelta))
	{
		FRotator CurrRot = CachedTurretMesh->GetRelativeRotation();
		CurrRot.Yaw += YawDelta;
		CachedTurretMesh->SetRelativeRotation(CurrRot);
	}

	// 배럴 상하 회전
	if (CachedBarrelPivotComp && !FMath::IsNearlyZero(PitchDelta))
	{
		FRotator CurrRot = CachedBarrelPivotComp->GetRelativeRotation();
		
		float TargetPitch = CurrRot.Pitch + PitchDelta;

		TargetPitch = FMath::Clamp(TargetPitch, -35.0f, 50.0f);

		CurrRot.Pitch = TargetPitch;
		CachedBarrelPivotComp->SetRelativeRotation(CurrRot);
		CurrentPitch = TargetPitch;
	}
}

void ABaseTank::FireCannon()
{
	if (!ProjectileClass) return;

	FVector SpawnLocation = FVector::ZeroVector;
	FVector ShootDirection = FVector::ForwardVector;

	if (CachedBarrelMesh && CachedBarrelMesh->DoesSocketExist(TEXT("Muzzle")))
	{
		SpawnLocation = CachedBarrelMesh->GetSocketLocation(TEXT("Muzzle"));
		
		FRotator MuzzleWorldRot = CachedBarrelMesh->GetSocketRotation(TEXT("Muzzle"));
		ShootDirection = MuzzleWorldRot.Vector();
	}
	else
	{
		SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 150.f) + FVector(0, 0, 100.f);
		ShootDirection = GetActorForwardVector();
	}

	SpawnLocation += (ShootDirection * 10.f); 

	// 디버그 화살표 그리기
	DrawDebugDirectionalArrow(GetWorld(), SpawnLocation, SpawnLocation + (ShootDirection * 100.f), 30.f, FColor::Cyan, false, 5.f);

	FTransform SpawnTransform(ShootDirection.Rotation(), SpawnLocation);
	ABaseProjectile* Projectile = GetWorld()->SpawnActorDeferred<ABaseProjectile>(
		ProjectileClass, SpawnTransform, this, GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (Projectile)
	{
		this->MoveIgnoreActorAdd(Projectile);
		Projectile->SetActorEnableCollision(false);
		
		SetTankPhase(ETankPhase::Wait);
		Projectile->OnExplosionHit.AddDynamic(this, &ABaseTank::OnProjectileExploded);

		Projectile->FireInDirection(ShootDirection, TankBasePower, FVector::ZeroVector);
		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransform);
		
		Projectile->SetActorEnableCollision(true);
	}
}

/// ----- 포탄이 터지면 게임모드에 턴 종료를 알림
void ABaseTank::OnProjectileExploded(FVector HitLocation, float Radius)
{
	if (ATurnGameMode* GameMode = GetWorld()->GetAuthGameMode<ATurnGameMode>())
	{
		GameMode->EndCurrentTurn();
	}
}

float ABaseTank::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);
	
	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
	
	if (CurrentHealth <= 0.0f)
	{
		// 사망 처리 (파괴 이펙트나 게임 모드에 사망 알림을 보내기 등)
	}
	
	return ActualDamage;
}

void ABaseTank::OnTurnStart()
{
    SetTankPhase(ETankPhase::Aim);
}

void ABaseTank::OnTurnEnd()
{
    SetTankPhase(ETankPhase::Wait);
}