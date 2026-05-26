#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuUIWidget.generated.h"

UCLASS()
class ONETWOSHOOT_API UMainMenuUIWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	class UButton* StartGameButton;
	
	UPROPERTY(meta = (BindWidget))
	class UButton* QuitGameButton;
	
	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnQuitClicked();
};
