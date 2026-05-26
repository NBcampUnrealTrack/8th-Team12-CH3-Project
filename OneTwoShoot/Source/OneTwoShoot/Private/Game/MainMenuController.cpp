#include "../Public/Game/MainMenuController.h"
#include "../Public/UI/MainMenuUIWidget.h"
#include "Blueprint/UserWidget.h"

void AMainMenuController::BeginPlay()
{
	Super::BeginPlay();
	
	bShowMouseCursor = true;
	
	FInputModeUIOnly InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputModeData);
	
	if (MenuUIClass)
	{
		MenuUIInstance = CreateWidget<UMainMenuUIWidget>(this, MenuUIClass);
		
		if (MenuUIInstance)
		{
			MenuUIInstance->AddToViewport();
		}
	}
}
