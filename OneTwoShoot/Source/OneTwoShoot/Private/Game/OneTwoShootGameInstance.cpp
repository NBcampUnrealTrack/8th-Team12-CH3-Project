// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/OneTwoShootGameInstance.h"

#include "Game/GameSave.h"
#include "Item/InventoryManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"


UOneTwoShootGameInstance::UOneTwoShootGameInstance()
{
	CurrentSave = nullptr;
	SaveSlotName = TEXT("OneTwoShoot_Save");
	SaveUserIndex = 0;

}

void UOneTwoShootGameInstance::Init()
{
	Super::Init();

	LoadProgress();

	if (!InventoryManager)
	{
		InventoryManager = NewObject<UInventoryManager>(this);

		TArray<FInventoryItemSaveData> EmptyData;
		InventoryManager->Initialize(EmptyData);

		InventoryManager->AddItem(TEXT("HealItem"), 1);
		InventoryManager->AssignToQuickSlot(0, TEXT("HealItem"));
	}
}

void UOneTwoShootGameInstance::StartNewRun()
{
	RunData = FRunData();
	RunData.bHasActiveRun = true;
	RunData.CurrentStageIndex = 0;

	SaveProgress();
	OpenCurrentStageLevel();
}

void UOneTwoShootGameInstance::OpenCurrentStageLevel()
{
	if (!StageLevels.IsValidIndex(RunData.CurrentStageIndex))
	{
		EndRun(true);
		return;
	}

	const TSoftObjectPtr<UWorld>& StageLevel = StageLevels[RunData.CurrentStageIndex];

	if (StageLevel.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenCurrentStageLevel failed: StageLevel is null."));
		return;
	}

	const FString StagePackageName = FPackageName::ObjectPathToPackageName(StageLevel.ToString());
	const FName StageLevelName(*StagePackageName);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Opening stage %d: %s"),
		RunData.CurrentStageIndex + 1,
		*StageLevelName.ToString()
	);

	UGameplayStatics::OpenLevel(this, StageLevelName);
}

void UOneTwoShootGameInstance::AdvanceToNextStage()
{
	if (!RunData.bHasActiveRun)
	{
		UE_LOG(LogTemp, Warning, TEXT("AdvanceToNextStage ignored: no active run."));
		return;
	}

	RunData.CurrentStageIndex++;

	if (!StageLevels.IsValidIndex(RunData.CurrentStageIndex))
	{
		EndRun(true);
		return;
	}

	SaveProgress();
	OpenCurrentStageLevel();
}

void UOneTwoShootGameInstance::RestartCurrentStage()
{
	if (!RunData.bHasActiveRun)
	{
		StartNewRun();
		return;
	}

	OpenCurrentStageLevel();
}

void UOneTwoShootGameInstance::EndRun(bool bCleared)
{
	RunData.bHasActiveRun = false;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Run ended. Cleared=%s"),
		bCleared ? TEXT("true") : TEXT("false")
	);

	SaveProgress();

	if (MainMenu.IsNull())
	{
		return;
	}

	const FString MainMenuPackageName = FPackageName::ObjectPathToPackageName(MainMenu.ToString());
	const FName MainMenuLevelName(*MainMenuPackageName);

	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}

bool UOneTwoShootGameInstance::LoadProgress()
{
	USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex);
	CurrentSave = Cast<UGameSave>(LoadedSave);

	if (!CurrentSave)
	{
		CreateFreshSave();
		return false;
	}

	ApplySaveToRuntimeData();
	return true;
}

bool UOneTwoShootGameInstance::SaveProgress()
{
	if (!CurrentSave)
	{
		CreateFreshSave();
	}

	ApplyRuntimeDataToSave();
	return CurrentSave && UGameplayStatics::SaveGameToSlot(CurrentSave, SaveSlotName, SaveUserIndex);
}

bool UOneTwoShootGameInstance::HasActiveRun() const
{
	return RunData.bHasActiveRun;
}

int32 UOneTwoShootGameInstance::GetCurrentStageIndex() const
{
	return RunData.CurrentStageIndex;
}

void UOneTwoShootGameInstance::SetCurrentStageIndex(int32 NewStageIndex)
{
	RunData.CurrentStageIndex = FMath::Max(0, NewStageIndex);
}

void UOneTwoShootGameInstance::CreateFreshSave()
{
	CurrentSave = Cast<UGameSave>(UGameplayStatics::CreateSaveGameObject(UGameSave::StaticClass()));
	ApplySaveToRuntimeData();
}

void UOneTwoShootGameInstance::ApplySaveToRuntimeData()
{
	if (!CurrentSave)
	{
		ProfileData = FPlayerProfileData();
		RunData = FRunData();
		return;
	}

	ProfileData = CurrentSave->ProfileData;
	RunData = CurrentSave->RunData;
}

void UOneTwoShootGameInstance::ApplyRuntimeDataToSave()
{
	if (!CurrentSave)
	{
		return;
	}

	CurrentSave->ProfileData = ProfileData;
	CurrentSave->RunData = RunData;
}
