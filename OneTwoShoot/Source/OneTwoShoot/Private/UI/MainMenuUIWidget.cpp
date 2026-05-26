#include "../Public/UI/MainMenuUIWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuUIWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &UMainMenuUIWidget::OnStartClicked);
	}
	
	if (QuitGameButton)
	{
		QuitGameButton->OnClicked.AddDynamic(this, &UMainMenuUIWidget::OnQuitClicked);
	}
}

void UMainMenuUIWidget::OnStartClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName("Level1"));
}

void UMainMenuUIWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, true);
}
