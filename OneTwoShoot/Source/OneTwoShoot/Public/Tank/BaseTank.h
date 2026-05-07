#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Public/Game/TankGameTypes.h" 
#include "BaseTank.generated.h"

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
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void UpdateAimAngle(float PitchDelta, float YawDelta);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ETankPhase CurrentPhase;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TankBasePower = 70.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aim")
	float CurrentPitch = 45.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aim")
	float CurrentYaw = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<ABaseProjectile> ProjectileClass;
};
