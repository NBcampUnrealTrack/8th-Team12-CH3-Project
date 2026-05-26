// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/BaseItem.h"
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

	UFUNCTION(BlueprintCallable, Category = "Run")
	void StartNewRun();

	UFUNCTION(BlueprintCallable, Category = "Run")
	int32 GetCurrentStageIndex() const;

	UFUNCTION(BlueprintCallable, Category = "Run")
	void OpenCurrentStageLevel();

	UFUNCTION(BlueprintCallable, Category = "Run")
	void AdvanceToNextStage();

	UFUNCTION(BlueprintCallable, Category = "Run")
	void RestartCurrentStage();

	UFUNCTION(BlueprintCallable, Category = "Run")
	void EndRun(bool bCleared);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run")
	TArray<TSoftObjectPtr<UWorld>> StageLevels;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run")
	TSoftObjectPtr<UWorld> MainMenu;

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadProgress();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveProgress();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool HasActiveRun() const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	void SetCurrentStageIndex(int32 NewStageIndex);

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FPlayerProfileData ProfileData;

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FRunData RunData;

	UPROPERTY() // 더미
	class UInventoryManager* InventoryManager;

	UPROPERTY(EditDefaultsOnly, Category = "Item System") //더미
	TMap<FName, TSubclassOf<UBaseItem>> ItemDatabase;

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
