#include "Game/MenuGameMode.h"

#include "Game/MainMenuController.h"

AMenuGameMode::AMenuGameMode()
{
	DefaultPawnClass = nullptr;
	
	PlayerControllerClass = AMainMenuController::StaticClass();
}
