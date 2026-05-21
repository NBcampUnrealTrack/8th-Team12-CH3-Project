#include "Tank/Player/PlayerTank.h"
#include "Tank/Player/Drone.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"

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
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxStepHeight = 85.0f; 

		MoveComp->SetWalkableFloorAngle(70.0f);
	}
}

void APlayerTank::BeginPlay()
{
	Super::BeginPlay();
	SetTankPhase(ETankPhase::Wait);
	bIsDroneView = false;
}

void APlayerTank::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &APlayerTank::Input_Move);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &APlayerTank::Input_Horizontal);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerTank::Input_Look);
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &APlayerTank::Input_ToggleCamera);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APlayerTank::Input_Fire);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &APlayerTank::Input_Aim);
	}
}

bool APlayerTank::CanMove() const
{
	/// 임시로 조준 상태에서 움직임 가능
	return (CurrentPhase == ETankPhase::Move || CurrentPhase == ETankPhase::Aim);
}

float APlayerTank::CalculateActiveSpeed() const
{
	return MoveSpeed; 
}

float APlayerTank::CalculateActiveRotationSpeed() const
{
	return MoveSpeed;
}

/// ----- 탱크를 움직이는 로직
void APlayerTank::Input_Move(const FInputActionValue& Value)
{
	float MoveValue = Value.Get<float>();
	
	if (CanMove() && (MoveValue != 0.0f))
	{
		AddMovementInput(GetActorForwardVector(), MoveValue * CalculateActiveSpeed());
	}
}

/// ----- 탱크를 회전하는 로직
void APlayerTank::Input_Horizontal(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	
	if (CanMove() && AxisValue != 0.0f)
	{
		float RotationSpeed = CalculateActiveRotationSpeed();
		float DeltaRotation = AxisValue * RotationSpeed * GetWorld()->GetDeltaSeconds();
		AddActorLocalRotation(FRotator(0.0f, DeltaRotation, 0.0f));
	}
}

/// ----- 캐릭터 시야 처리 (★ 드론 뷰일 때도 컨트롤러 회전 값을 그대로 반영)
void APlayerTank::Input_Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (GetController() != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

/// ----- 드론 뷰 <-> 탱크 뷰 전환 처리
void APlayerTank::ToggleCameraView(APlayerController* InPC)
{
	if (!DroneClass) return;

	APlayerController* PC = InPC ? InPC : Cast<APlayerController>(GetController());
	if (!PC) return;

	bIsDroneView = !bIsDroneView;

	if (bIsDroneView) EnterDroneMode(PC);
	else              ExitDroneMode(PC);
}

// 드론 모드 진입
void APlayerTank::EnterDroneMode(APlayerController* PC)
{
	if (!PC) return;

	// 기존 탱크의 시야 각도를 안전하게 백업
	SavedTankRotation = PC->GetControlRotation();

	FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 150.0f);
	FRotator SpawnRotation = SavedTankRotation;
	SpawnRotation.Pitch = -45.0f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	SpawnedDrone = GetWorld()->SpawnActor<ADrone>(DroneClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (SpawnedDrone)
	{
		SpawnedDrone->SetCreatorTank(this);
		PC->Possess(SpawnedDrone);
	}
}

// 탱크 모드 복귀
void APlayerTank::ExitDroneMode(APlayerController* PC)
{
	if (!PC) return;

	if (SpawnedDrone)
	{
		SpawnedDrone->Destroy();
		SpawnedDrone = nullptr;
	}
	
	PC->Possess(this);
	PC->SetControlRotation(SavedTankRotation);
}

void APlayerTank::Input_ToggleCamera()
{
	ToggleCameraView();
}

void APlayerTank::Input_Fire()
{
	if (CurrentPhase == ETankPhase::Aim || CurrentPhase == ETankPhase::Action)
	{
		FireCannon();
	}
}

/// ----- 탱크의 조준 기능 및 애니메이션
void APlayerTank::Input_Aim(const FInputActionValue& Value)
{
	if (CurrentPhase != ETankPhase::Aim) return;

	FVector2D AimVector = Value.Get<FVector2D>();
    
	float AimSpeed = 45.0f; 
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	float YawDelta = AimVector.X * AimSpeed * DeltaTime;
	float PitchDelta = AimVector.Y * AimSpeed * DeltaTime;

	UpdateAimAngle(PitchDelta, YawDelta);
}

void APlayerTank::OnTurnStart()
{
	Super::OnTurnStart();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MovingMappingContext, 0);
			Subsystem->AddMappingContext(CombatMappingContext, 0);
			Subsystem->AddMappingContext(LookMappingContext, 0);
		}
	}
}

void APlayerTank::OnTurnEnd()
{
	Super::OnTurnEnd();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(MovingMappingContext);
			Subsystem->RemoveMappingContext(CombatMappingContext);
			Subsystem->RemoveMappingContext(LookMappingContext);
		}
	}
}
