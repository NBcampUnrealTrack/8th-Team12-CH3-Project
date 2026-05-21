#include "Tank/Player/Drone.h"
#include "Tank/Player/PlayerTank.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ADrone::ADrone()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	RootComponent = SphereCollision;
	SphereCollision->SetSphereRadius(40.0f);
	SphereCollision->SetCollisionProfileName(TEXT("Pawn"));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->bUsePawnControlRotation = true;

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->MaxSpeed = 1000.0f;
	MovementComponent->Acceleration = 4000.0f;
	MovementComponent->Deceleration = 8000.0f;
}

void ADrone::BeginPlay()
{
	Super::BeginPlay();
}

void ADrone::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DroneMappingContext, 0);
		}
	}
}

void ADrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &ADrone::Input_MoveForward);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ADrone::Input_MoveRight);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADrone::Input_Look);
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &ADrone::Input_ToggleCamera);
	}
}

void ADrone::Input_MoveForward(const FInputActionValue& Value)
{
	float MoveValue = Value.Get<float>();
	if (MoveValue != 0.0f && GetController())
	{
		const FRotator ControlRot = GetController()->GetControlRotation();
		const FVector Direction = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, MoveValue);
	}
}

void ADrone::Input_MoveRight(const FInputActionValue& Value)
{
	float MoveValue = Value.Get<float>();
	if (MoveValue != 0.0f && GetController())
	{
		const FRotator ControlRot = GetController()->GetControlRotation();
		const FVector Direction = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, MoveValue);
	}
}

void ADrone::Input_Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (GetController() != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ADrone::Input_ToggleCamera(const FInputActionValue& Value)
{
	if (MyCreatorTank)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			MyCreatorTank->ToggleCameraView(PC);
		}
	}
}