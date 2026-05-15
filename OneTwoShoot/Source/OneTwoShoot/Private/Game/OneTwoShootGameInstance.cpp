// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/OneTwoShootGameInstance.h"

#include "Game/GameSave.h"
#include "Kismet/GameplayStatics.h"

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
}

void UOneTwoShootGameInstance::StartNewRun()
{
	RunData = FRunData();
	RunData.bHasActiveRun = true;
	RunData.CurrentStageIndex = 0;

	SaveProgress();
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
