#pragma once

#include "CoreMinimal.h"
#include "BaseTank.h"
#include "PlayerTank.generated.h"

UCLASS()
class ONETWOSHOOT_API APlayerTank : public ABaseTank
{
	GENERATED_BODY()
	
public:
	APlayerTank();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* MovingMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* LookMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveForwardAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* TurnAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* LookAction;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* Camera;
	
	/// ----- 바퀴 충돌 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LeftWheelCollision;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* RightWheelCollision;

	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);
	void Input_Move(const struct FInputActionValue& Value);
	void Input_Turn(const struct FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	float CalculateActiveSpeed() const;
	float CalculateActiveRotationSpeed() const;
	bool CanMove() const;
	
	virtual void BeginPlay() override;
};
