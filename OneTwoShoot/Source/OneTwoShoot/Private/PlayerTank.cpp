#include "PlayerTank.h"
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
	
	CurrentPhase = ETankPhase::Move;

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MovingMappingContext, 0);
			Subsystem->AddMappingContext(LookMappingContext, 0); 
		}
	}
}

void APlayerTank::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &APlayerTank::Input_Move);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &APlayerTank::Input_Turn);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerTank::Input_Look);
	}
}

// 1. 이동 가능 상태인지 확인
bool APlayerTank::CanMove() const
{
	return (CurrentPhase == ETankPhase::Move);
}

// 2. 현재 속도 계산
float APlayerTank::CalculateActiveSpeed() const
{
	/// ----- 추후 버프, 디버프 로직 추가하면 됨
	return TankBasePower; 
}

// 3. 현재 회전 속도 계산
float APlayerTank::CalculateActiveRotationSpeed() const
{
	/// ----- 혹시 회전 속도도 버프, 디버프에 따라 영향을 줄지 몰라 미리 만들어둠
	return TankBasePower; 
}

// 4. 실제 이동 처리
void APlayerTank::Input_Move(const FInputActionValue& Value)
{
	float MoveValue = Value.Get<float>();

	if (CanMove() && (MoveValue != 0.0f))
	{
		AddMovementInput(GetActorForwardVector(), MoveValue * CalculateActiveSpeed());
	}
}

// 5. 실제 회전 처리
void APlayerTank::Input_Turn(const FInputActionValue& Value)
{
	float TurnValue = Value.Get<float>();

	if (TurnValue != 0.0f)
	{
		float RotationSpeed = CalculateActiveRotationSpeed();
        
		/// ----- 입력값 * 초당 회전도 * 프레임 시간
		float DeltaRotation = TurnValue * RotationSpeed * GetWorld()->GetDeltaSeconds();
		AddActorLocalRotation(FRotator(0.0f, DeltaRotation, 0.0f));
	}
}


// 6. 캐릭터 시야 처리
void APlayerTank::Input_Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (GetController() != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}