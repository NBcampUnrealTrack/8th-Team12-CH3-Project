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
	
protected:
	/// ----- 입력 매핑
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
	
	/// ----- 탑뷰 카메라 매핑
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ToggleCameraAction;
	/// ----- 드론 이동 속도
	UPROPERTY(EditAnywhere, Category = "Input|Drone")
	float DroneMoveSpeed = 1000.0f;
	/// ----- 드론 뷰일 때 카메라가 중심(탱크)에서 얼마나 떨어져 있는지 저장
	UPROPERTY(VisibleAnywhere, Category = "Input|Drone")
	FVector DroneOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "Input|Drone")
	class UInputAction* ZoomAction;
	// 휠 한 칸당 줌 속도
	UPROPERTY(EditAnywhere, Category = "Input|Camera")
	float ZoomSpeed = 100.0f;
	// 최소/최대 줌 거리
	UPROPERTY(EditAnywhere, Category = "Input|Camera")
	float MinZoomLength = 300.0f;
	UPROPERTY(EditAnywhere, Category = "Input|Camera")
	float MaxZoomLength = 4000.0f;
	
	/// ----- 카메라 매핑
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* Camera;
	
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
	void MoveTank(float Value);
	void MoveDroneForward(float Value);
	void MoveDroneRight(float Value);
	void Input_Move(const struct FInputActionValue& Value);
	void RotateTank(float Value);
	void Input_Horizontal(const struct FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void ToggleCameraView();
	void Input_Zoom(const struct FInputActionValue& Value);
	virtual void BeginPlay() override;
};
