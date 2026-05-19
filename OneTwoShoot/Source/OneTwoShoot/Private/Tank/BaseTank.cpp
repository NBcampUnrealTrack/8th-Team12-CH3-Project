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
    if (!ProjectileClass) return;

    FRotator CameraRot = GetControlRotation();
    FVector ForwardDir = FRotator(0.f, CameraRot.Yaw, 0.f).Vector();
    FVector RightDir = FRotator(0.f, CameraRot.Yaw + 90.f, 0.f).Vector();
    FVector ShootDirection = ForwardDir.RotateAngleAxis(-CurrentPitch, RightDir);

    FVector SpawnLocation = FVector::ZeroVector;
    
    TArray<UStaticMeshComponent*> MeshComps;
    GetComponents<UStaticMeshComponent>(MeshComps);
    
    UStaticMeshComponent* BarrelComp = nullptr;
	for (UStaticMeshComponent* CurrMesh : MeshComps)
	{
        if (CurrMesh->GetName() == TEXT("Barrel"))
        {
            BarrelComp = CurrMesh;
            break;
        }
    }

    if (BarrelComp && BarrelComp->DoesSocketExist(TEXT("Muzzle")))
    {
        SpawnLocation = BarrelComp->GetSocketLocation(TEXT("Muzzle"));
    }
    else
    {
        SpawnLocation = GetActorLocation() + (ShootDirection * 150.f) + FVector(0, 0, 100.f);
    }

    SpawnLocation += (ShootDirection * 10.f); 

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