#include "../Public/Game/TankPlayerController.h"
#include "../Public/UI/TankUIWidget.h"
#include "../Public/Tank/BaseTank.h"

void ATankPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UIWidgetClass != nullptr)
	{
		UIWidgetInstance = CreateWidget<UTankUIWidget>(this, UIWidgetClass);
		
		if (UIWidgetInstance != nullptr)
		{
			UIWidgetInstance->AddToViewport();
			
			if (ABaseTank* ControlledTank = Cast<ABaseTank>(GetPawn()))
			{
				UIWidgetInstance->BindToTank(ControlledTank);
			}
		}
	}
}

void ATankPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	ABaseTank* PossessedTank = Cast<ABaseTank>(InPawn);
	
	if (PossessedTank && UIWidgetInstance)
	{
		UIWidgetInstance->BindToTank(PossessedTank);
	}
}