#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Public/Game/TankGameTypes.h" 
#include "BaseTank.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);

class ABaseProjectile;

UCLASS(Abstract)
class ONETWOSHOOT_API ABaseTank : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseTank();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void SetTankPhase(ETankPhase NewPhase);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void FireCannon();
	
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }
	
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void UpdateAimAngle(float PitchDelta, float YawDelta);
	
	UFUNCTION()
	void OnProjectileExploded(FVector HitLocation, float Radius);
	
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnHealthChangedSignature OnHealthChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ETankPhase CurrentPhase;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TankBasePower = 1500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MoveSpeed = 70.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aim")
	float CurrentPitch = 45.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aim")
	float CurrentYaw = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<ABaseProjectile> ProjectileClass;
};
