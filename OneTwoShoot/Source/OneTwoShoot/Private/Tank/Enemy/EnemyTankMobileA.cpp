#include "../Public/Tank/Enemy/EnemyTankMobileA.h"
#include "../Public/Tank/BaseProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

AEnemyTankMobileA::AEnemyTankMobileA()
{
    AttackRange = 1500.f;
    LaunchPower = 4000.f;
}

void AEnemyTankMobileA::BeginPlay()
{
    Super::BeginPlay();

    TArray<USceneComponent*> SceneComps;
    GetComponents<USceneComponent>(SceneComps);

    for (USceneComponent* CurrComp : SceneComps)
    {   
        if (CurrComp)
        {
            if (CurrComp->GetName() == TEXT("BarrelPivot"))
            {
                BarrelPivotComp = CurrComp;
            }
            else if (CurrComp->GetName() == TEXT("FirePoint"))
            {
                FirePivotComp = CurrComp;
            }
        }
    }
}

void AEnemyTankMobileA::Aim()
{
	Super::Aim();

	if (BarrelPivotComp && FirePivotComp && TargetPlayer)
	{
		FVector TossVelocity;
		bool bSuccess = UGameplayStatics::SuggestProjectileVelocity(
			this,
			TossVelocity,
			FirePivotComp->GetComponentLocation(),
			TargetPlayer->GetActorLocation(),
            LaunchPower,
			false, 0.f, 0.f,
			ESuggestProjVelocityTraceOption::DoNotTrace
		);

		if (bSuccess)
		{
			FRotator PerfectAimRot = TossVelocity.Rotation();
			FRotator BarrelRelRot = BarrelPivotComp->GetRelativeRotation();

			float TargetPitch = PerfectAimRot.Pitch - GetActorRotation().Pitch;
			TargetPitch = FMath::Clamp(TargetPitch, -35.0f, 50.0f);

			BarrelRelRot.Pitch = TargetPitch;
			BarrelPivotComp->SetRelativeRotation(BarrelRelRot);
		}
	}
}

void AEnemyTankMobileA::Fire()
{
    if (!ProjectileClass) return;
    if (!FirePivotComp) return;

    FVector SpawnLocation = FirePivotComp->GetComponentLocation();
    FRotator SpawnRotation = FRotator(CurrentAimAngle, GetActorRotation().Yaw, 0.f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    UE_LOG(LogTemp, Warning, TEXT("발사!"));

    ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(
        ProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

	if (FirePivotComp)
	{
		if (MuzzleFlashEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				MuzzleFlashEffect,
				SpawnLocation,
				SpawnRotation
			);
		}

		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				SpawnLocation
			);
		}

		UE_LOG(LogTemp, Warning, TEXT("[%s] 발사 이펙트 및 사운드 출력 완료!"), *GetName());
	}

    if (Projectile)
    {
        FVector LaunchDirection = SpawnRotation.Vector();

        FVector WindForce = FVector::ZeroVector;

        Projectile->FireInDirection(LaunchDirection, LaunchPower, WindForce);

        UE_LOG(LogTemp, Warning, TEXT("투사체 발사 성공!"));
    }
}