#pragma once

#include "CoreMinimal.h"
#include "Tank/BaseTank.h"
#include "Item/BaseItem.h"
#include "UObject/NoExportTypes.h"
#include "Game/GameSaveTypes.h"
#include "InventoryManager.generated.h"

UCLASS()
class ONETWOSHOOT_API UInventoryManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(const TArray<FInventoryItemSaveData>& SavedItems);
	void AddItem(FName ItemId, int32 Count);
	void UseItem(FName ItemId, ABaseTank* UserTank);
	void UseItemAtSlot(int32 SlotIndex, class AActor* UserActor);
	void AssignToQuickSlot(int32 SlotIndex, FName ItemId);
	void UseQuickSlotItem(int32 SlotIndex, ABaseTank* UserTank);

	TArray<FInventoryItemSaveData> GetInventoryData() const { return InventoryItems; }

private:
	UPROPERTY()
	TArray<FInventoryItemSaveData> InventoryItems;
	UPROPERTY()
	TArray<FName> QuickSlotItems;
};