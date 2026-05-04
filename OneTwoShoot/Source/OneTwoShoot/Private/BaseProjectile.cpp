#include "BaseProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseProjectile::ABaseProjectile()
{
 	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->SetSphereRadius(15.0f);
	CollisionComponent->BodyInstance.SetCollisionProfileName("BlockAllDynamic");
	CollisionComponent->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
	RootComponent = CollisionComponent;
	
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	
	ConstantWindAcceleration = FVector::ZeroVector;
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
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
		GetInstigatorController()
		);
	
	// OnExplosionHit.Broadcast(Hit.ImpactPoint, FinalRadius);
	
	Destroy();
}
