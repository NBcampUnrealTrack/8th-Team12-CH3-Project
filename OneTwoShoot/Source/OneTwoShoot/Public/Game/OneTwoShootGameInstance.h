// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Game/GameSaveTypes.h"
#include "OneTwoShootGameInstance.generated.h"

class UGameSave;

UCLASS()
class ONETWOSHOOT_API UOneTwoShootGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UOneTwoShootGameInstance();

	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Save")
	void StartNewRun();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadProgress();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveProgress();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool HasActiveRun() const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	int32 GetCurrentStageIndex() const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	void SetCurrentStageIndex(int32 NewStageIndex);

	UPROPERTY(BlueprintReadOnly, Category = "Save|Profile")
	FPlayerProfileData ProfileData;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Run")
	FRunData RunData;

private:
	UPROPERTY()
	UGameSave* CurrentSave;

	UPROPERTY(EditDefaultsOnly, Category = "Save")
	FString SaveSlotName;

	UPROPERTY(EditDefaultsOnly, Category = "Save")
	int32 SaveUserIndex;

	void CreateFreshSave();
	void ApplySaveToRuntimeData();
	void ApplyRuntimeDataToSave();
	
};
