// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/GameSaveTypes.h"
#include "GameFramework/SaveGame.h"
#include "GameSave.generated.h"

UCLASS()
class ONETWOSHOOT_API UGameSave : public USaveGame
{
	GENERATED_BODY()

public:
	UGameSave();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save|Slot")
	int32 SaveVersion;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save|Profile")
	FPlayerProfileData ProfileData;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save|Run")
	FRunData RunData;
};
