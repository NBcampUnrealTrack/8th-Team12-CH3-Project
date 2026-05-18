#include "../Public/Tank/Player/PlayerTank.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

APlayerTank::APlayerTank()
{
	LeftWheelCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWheelCollision"));
	LeftWheelCollision->SetupAttachment(GetCapsuleComponent());
	LeftWheelCollision->SetCollisionProfileName(TEXT("BlockAll"));
	
	RightWheelCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWheelCollision"));
	RightWheelCollision->SetupAttachment(GetCapsuleComponent());
	RightWheelCollision->SetCollisionProfileName(TEXT("BlockAll"));
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->TargetArmLength = 500.0f;
	bUseControllerRotationYaw = false; 
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}

void APlayerTank::BeginPlay()
{
	Super::BeginPlay();
	
	//CurrentPhase = ETankPhase::Aim;
	SetTankPhase(ETankPhase::Wait);
	bIsDroneView = false;

	//OnTurnStart와 OnTurnEnd 함수 추가에 따라 IA 매핑은 해당 함수들이 관리할 예정.
	//if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	//{
	//	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	//	{
	//		Subsystem->AddMappingContext(MovingMappingContext, 0);
	//		Subsystem->AddMappingContext(LookMappingContext, 0); 
	//		Subsystem->AddMappingContext(CombatMappingContext, 0); 
	//	}
	//}
}

void APlayerTank::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &APlayerTank::Input_Move);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &APlayerTank::Input_Horizontal);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerTank::Input_Look);
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &APlayerTank::ToggleCameraView);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &APlayerTank::Input_Zoom);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APlayerTank::Input_Fire);
	}
}

/// ----- 이동 가능 상태인지 확인
bool APlayerTank::CanMove() const
{
	// return (CurrentPhase == ETankPhase::Move); ----- 임시로 Aim상태에서도 이동 가능하게 변경
	return (CurrentPhase == ETankPhase::Move || CurrentPhase == ETankPhase::Aim);
}

/// ----- 현재 속도 계산
float APlayerTank::CalculateActiveSpeed() const
{
	/// ----- 추후 버프, 디버프 로직 추가하면 됨
	return MoveSpeed; 
}

/// ----- 현재 회전 속도 계산
float APlayerTank::CalculateActiveRotationSpeed() const
{
	/// ----- 혹시 회전 속도도 버프, 디버프에 따라 영향을 줄지 몰라 미리 만들어둠
	return MoveSpeed;
}

/// ----- 탱크를 움직이는 로직
void APlayerTank::MoveTank(float Value)
{
	if (CanMove() && (Value != 0.0f))
	{
		AddMovementInput(GetActorForwardVector(), Value * CalculateActiveSpeed());
	}
}

/// ----- 드론 앞뒤 이동 처리
void APlayerTank::MoveDroneForward(float Value)
{
	if (Value == 0.0f) return;

	FVector WorldForward = FVector(1.0f, 0.0f, 0.0f);

	DroneOffset += WorldForward * Value * DroneMoveSpeed * GetWorld()->GetDeltaSeconds();

	SpringArm->TargetOffset = DroneOffset;
}

/// ----- 드론 좌우 이동 처리
void APlayerTank::MoveDroneRight(float Value)
{
	if (Value == 0.0f) return;

	FVector WorldRight = FVector(0.0f, 1.0f, 0.0f);

	DroneOffset += WorldRight * Value * DroneMoveSpeed * GetWorld()->GetDeltaSeconds();

	SpringArm->TargetOffset = DroneOffset;
}

/// ----- 실제 이동 처리
void APlayerTank::Input_Move(const FInputActionValue& Value)
{
	float MoveValue = Value.Get<float>();
	if (bIsDroneView)
	{
		MoveDroneForward(MoveValue);
	}
	else
	{
		MoveTank(MoveValue);
	}
}

/// ----- 탱크 회전 로직
void APlayerTank::RotateTank(float Value)
{
	float RotationSpeed = CalculateActiveRotationSpeed();
        
	/// ----- 입력값 * 초당 회전도 * 프레임 시간
	float DeltaRotation = Value * RotationSpeed * GetWorld()->GetDeltaSeconds();
	AddActorLocalRotation(FRotator(0.0f, DeltaRotation, 0.0f));
}

/// ----- 실제 회전 처리
void APlayerTank::Input_Horizontal(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	if (AxisValue == 0.0f) return;

	if (bIsDroneView)
	{
		MoveDroneRight(AxisValue);
	}
	else
	{
		RotateTank(AxisValue);
	}
}

/// ----- 캐릭터 시야 처리
void APlayerTank::Input_Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (GetController() != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

/// ----- 캐릭터 시야 전환 처리
void APlayerTank::ToggleCameraView()
{
	bIsDroneView = !bIsDroneView;

	if (bIsDroneView)
	{
		SpringArm->SetUsingAbsoluteRotation(true);
        
		SpringArm->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));

		SpringArm->TargetArmLength = 2000.0f; 
		SpringArm->bUsePawnControlRotation = false; 
        
		DroneOffset = FVector::ZeroVector;
		SpringArm->TargetOffset = DroneOffset;
	}
	else
	{
		SpringArm->SetUsingAbsoluteRotation(false);
        
		SpringArm->TargetArmLength = 500.0f;
		SpringArm->bUsePawnControlRotation = true; 
		SpringArm->TargetOffset = FVector::ZeroVector;
	}
}

/// ----- 캐릭터 시야 확대/축소 처리
void APlayerTank::Input_Zoom(const FInputActionValue& Value)
{
	if (!bIsDroneView) return; 

	float ZoomValue = Value.Get<float>();
	if (ZoomValue != 0.0f)
	{
		float NewLength = SpringArm->TargetArmLength + (ZoomValue * ZoomSpeed * -1.0f);
		SpringArm->TargetArmLength = FMath::Clamp(NewLength, MinZoomLength, MaxZoomLength);
	}
}

void APlayerTank::Input_Fire()
{
	if (CurrentPhase == ETankPhase::Aim || CurrentPhase == ETankPhase::Action)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire!"));
		FireCannon();
	}
}

void APlayerTank::OnTurnStart()
{
	Super::OnTurnStart();

	// 플레이어 차례가 되었으므로 조작 권한(Mapping Context)을 부여합니다.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MovingMappingContext, 0);
			Subsystem->AddMappingContext(CombatMappingContext, 0);
			Subsystem->AddMappingContext(LookMappingContext, 0);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("플레이어 조작 권한이 활성화되었습니다."));
}

void APlayerTank::OnTurnEnd()
{
	Super::OnTurnEnd();

	// 플레이어 차례가 끝났으므로 조작 권한을 완전히 회수합니다. (적 턴 내부 조작 방지)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(MovingMappingContext);
			Subsystem->RemoveMappingContext(CombatMappingContext);
			Subsystem->RemoveMappingContext(LookMappingContext);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("플레이어 조작 권한이 비활성화되었습니다."));
}