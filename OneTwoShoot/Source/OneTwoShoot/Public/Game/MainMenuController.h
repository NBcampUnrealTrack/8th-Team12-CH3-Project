#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuController.generated.h"

UCLASS()
class ONETWOSHOOT_API AMainMenuController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UMainMenuUIWidget> MenuUIClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UMainMenuUIWidget* MenuUIInstance;
	
protected:
	virtual void BeginPlay() override;
};
