#pragma once

#include "CoreMinimal.h"
#include "../Public/Tank/BaseTank.h"
#include "PlayerTank.generated.h"

UCLASS()
class ONETWOSHOOT_API APlayerTank : public ABaseTank
{
	GENERATED_BODY()
	
public:
	APlayerTank();
	void ToggleCameraView(class APlayerController* InPC = nullptr);
	void EnterDroneMode(class APlayerController* PC);
	void ExitDroneMode(class APlayerController* PC);
	
protected:
	/// ----- 입력 매핑
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* MovingMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* LookMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* CombatMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveForwardAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* TurnAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* FireAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* AimAction;
	
	/// ----- 탑뷰 카메라 매핑
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ToggleCameraAction;
	
	/// ----- 카메라 매핑
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* Camera;
	// 드론으로 진입 하기 전의 탱크 시야를 저장하는 변수
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	FRotator SavedTankRotation = FRotator::ZeroRotator;
	
	
	/// ----- 드론 매핑
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Drone")
	TSubclassOf<class ADrone> DroneClass;
	// 현재 월드에 소환된 드론을 기억하고 지우기 위한 포인터 변수
	UPROPERTY(VisibleAnywhere, Category = "Input|Drone")
	class ADrone* SpawnedDrone;
	
	/// ----- 바퀴 충돌 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LeftWheelCollision;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* RightWheelCollision;

	/// ----- 변수 묶음
	float CalculateActiveSpeed() const;
	float CalculateActiveRotationSpeed() const;
	bool CanMove() const;
	bool bIsDroneView = false;
	
	/// ----- 함수 묶음
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);
	void Input_Move(const struct FInputActionValue& Value);
	void Input_Horizontal(const struct FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_Fire();
	void Input_ToggleCamera();
	void Input_Aim(const struct FInputActionValue& Value);
	virtual void BeginPlay() override;
	virtual void OnTurnStart() override;
	virtual void OnTurnEnd() override;
};
