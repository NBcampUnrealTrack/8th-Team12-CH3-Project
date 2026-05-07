#include "../Public/Tank/BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseProjectile::ABaseProjectile()
{
 	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->SetSphereRadius(15.0f);
	CollisionComponent->BodyInstance.SetCollisionProfileName("BlockAllDynamic");
	CollisionComponent->BodyInstance.bUseCCD = true;
	CollisionComponent->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
	RootComponent = CollisionComponent;
	
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->bSweepCollision = true;
	
	ConstantWindAcceleration = FVector::ZeroVector;

	UE_LOG(LogTemp, Warning, TEXT("포탄 생성 완료!"));
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 발사 주체(Owner)가 설정되어 있다면, 그 액터와는 충돌을 무시하도록 설정
	if (GetOwner())
	{
		CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
	}
}

void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ProjectileMovement && ProjectileMovement->Velocity.SizeSquared() > 0.0f)
	{
		ProjectileMovement->Velocity += ConstantWindAcceleration * DeltaTime;
	}
}

void ABaseProjectile::FireInDirection(const FVector& ShootDirection, float ShootPower, FVector StageWindForce)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = ShootDirection * ShootPower;
		ConstantWindAcceleration = StageWindForce;
	}
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 1. 자기 자신과 부딪혔거나, 발사 주체(Owner)와 부딪혔다면 무시
	if (OtherActor == nullptr || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	float FinalRadius = BaseExplosionRadius + (ProjectileDamage * RadiusPerDamage);
	
		UGameplayStatics::ApplyRadialDamageWithFalloff(
			this, 
			ProjectileDamage, 
			10.0f,
			Hit.ImpactPoint,
			FinalRadius,
			FinalRadius,
			1.0f, 
			nullptr,
			TArray<AActor*>(),
			this,
			GetInstigatorController(),
			ECC_Visibility
			);
		
		if (OnExplosionHit.IsBound())
		{
			OnExplosionHit.Broadcast(
				Hit.ImpactPoint,
				FinalRadius
			);
		}
	
	
	Destroy();
}
