#include "Tank/Player/PlayerTank.h"
#include "Tank/Player/Drone.h"
#include "Item/InventoryManager.h"
#include "Game/OneTwoShootGameInstance.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerController.h"
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
	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->bUsePawnControlRotation = false;
	FirstPersonCamera->SetActive(false);
}

void APlayerTank::BeginPlay()
{
	Super::BeginPlay();
	SetTankPhase(ETankPhase::Wait);
	
	OnPlayerPhaseChanged.Broadcast(ETankPhase::Wait);
	
	bIsDroneView = false;
	UOneTwoShootGameInstance* GI = Cast<UOneTwoShootGameInstance>(GetGameInstance());
	if (GI)
	{
		InventoryManager = GI->InventoryManager;
	}
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
		EnhancedInputComponent->BindAction(UseItem1Action, ETriggerEvent::Started, this, &APlayerTank::Input_UseItem1);
		EnhancedInputComponent->BindAction(UseItem2Action, ETriggerEvent::Started, this, &APlayerTank::Input_UseItem2);
		EnhancedInputComponent->BindAction(UseItem3Action, ETriggerEvent::Started, this, &APlayerTank::Input_UseItem3);
		EnhancedInputComponent->BindAction(UseItem4Action, ETriggerEvent::Started, this, &APlayerTank::Input_UseItem4);
		if (ConfirmPhaseAction)
		{
			EnhancedInputComponent->BindAction(ConfirmPhaseAction, ETriggerEvent::Started, this, &APlayerTank::Input_ConfirmPhase);
		}
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
	if (CurrentPhase != ETankPhase::Move) return;
	
	float MoveValue = Value.Get<float>();
	
	if (MoveValue != 0.0f)
	{
		float CurrentDistance = FVector::DistXY(TurnStartLocation, GetActorLocation());
		
		if (CurrentDistance >= MaxMoveDistance)
		{
			FVector ToCenter = (TurnStartLocation - GetActorLocation()).GetSafeNormal();
			
			FVector MoveDirection = GetActorForwardVector() * MoveValue;
			
			if (FVector::DotProduct(ToCenter, MoveDirection) < 0.0f)
			{
				return;
			}
		}
		
		AddMovementInput(GetActorForwardVector(), MoveValue * CalculateActiveSpeed());
	}
}

void APlayerTank::Input_ConfirmPhase()
{
	if (CurrentPhase == ETankPhase::Move)
	{
		SetTankPhase(ETankPhase::Aim);
		
		OnPlayerPhaseChanged.Broadcast(ETankPhase::Aim);
		
		Camera->SetActive(false);
		
		if (FirstPersonCamera)
		{
			FirstPersonCamera->SetActive(true);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("조준 단계 진입"));
	}
}

/// ----- 탱크를 회전하는 로직
void APlayerTank::Input_Horizontal(const FInputActionValue& Value)
{
	if (CurrentPhase != ETankPhase::Move) return;
	
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

/// ----- 드론 뷰 <-> 탱크 뷰 전환 처리
void APlayerTank::ToggleCameraView(APlayerController* InPC)
{
	if (!DroneClass) return;
	
	if (!CanMove()) return;
	APlayerController* PC = InPC ? InPC : Cast<APlayerController>(GetController());
	if (!PC) return;

	bIsDroneView = !bIsDroneView;

	if (bIsDroneView) EnterDroneMode(PC);
	else              ExitDroneMode(PC);
}

void APlayerTank::Input_ToggleCamera()
{
	ToggleCameraView();
}

void APlayerTank::Input_Fire()
{
	if (CurrentPhase == ETankPhase::Aim)
	{
		SetTankPhase(ETankPhase::Action);
		
		OnPlayerPhaseChanged.Broadcast(ETankPhase::Action);
		
		FireCannon();
		
		if (FirstPersonCamera)
		{
			FirstPersonCamera->SetActive(false);
		}
		
		Camera->SetActive(true);
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

void APlayerTank::Input_UseItem1()
{
	if (InventoryManager)
	{
		InventoryManager->UseItemAtSlot(0, this);
	}
}
void APlayerTank::Input_UseItem2()
{
	if (InventoryManager)
	{
		InventoryManager->UseItemAtSlot(1, this);
	}
}
void APlayerTank::Input_UseItem3()
{
	if (InventoryManager)
	{
		InventoryManager->UseItemAtSlot(2, this);
	}
}
void APlayerTank::Input_UseItem4()
{
	if (InventoryManager)
	{
		InventoryManager->UseItemAtSlot(3, this);
	}
}

void APlayerTank::OnTurnStart()
{
	Super::OnTurnStart();
	
	TurnStartLocation = GetActorLocation();
	
	// 디버그용 이동 거리 시각화
	// DrawDebugCylinder(GetWorld(), TurnStartLocation, TurnStartLocation + FVector(0, 0, 10), MaxMoveDistance, 32, FColor::Green, false, 10.0f);
	
	SetTankPhase(ETankPhase::Move);
	Camera->SetActive(true);
	
	OnPlayerPhaseChanged.Broadcast(ETankPhase::Move);
	
	if (FirstPersonCamera)
	{
		FirstPersonCamera->SetActive(false);
	}
	
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
