// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSaveTypes.generated.h"

USTRUCT(BlueprintType)
struct ONETWOSHOOT_API FInventoryItemSaveData // 일단 더미용으로 생성만 해 둔 데이터
{
	GENERATED_BODY()

	FInventoryItemSaveData()
		: ItemId(NAME_None)
		, Count(1)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Inventory")
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Inventory")
	int32 Count;
};

USTRUCT(BlueprintType)
struct ONETWOSHOOT_API FPlayerProfileData // 세이브 프로필에 저장되는, 승계되는 데이터
{
	GENERATED_BODY()

	FPlayerProfileData()
	{
		UnlockedTanks.Add(TEXT("DefaultTank"));
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Profile")
	TArray<FName> UnlockedTanks;
};

USTRUCT(BlueprintType)
struct ONETWOSHOOT_API FRunData
{
	GENERATED_BODY()

	FRunData()
		: bHasActiveRun(false)
		, CurrentStageIndex(0)
		, SelectedTankId(TEXT("DefaultTank"))
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Run")
	bool bHasActiveRun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Run")
	int32 CurrentStageIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Run")
	FName SelectedTankId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Run")
	TArray<FInventoryItemSaveData> InventoryItems;
};