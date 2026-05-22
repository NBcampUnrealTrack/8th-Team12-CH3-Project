#include "Item/InventoryManager.h"
#include "Game/OneTwoShootGameInstance.h"

void UInventoryManager::Initialize(const TArray<FInventoryItemSaveData>& SavedItems)
{
	InventoryItems = SavedItems;
	QuickSlotItems.SetNumZeroed(4);

	// 테스트용 아이템 4개 추가
	AddItem(TEXT("HealItem"), 5);
	AddItem(TEXT("BuffItem"), 3);
	AddItem(TEXT("AttackItem"), 2);
	AddItem(TEXT("ShieldItem"), 1);

	// 퀵슬롯에 각각 할당 (인덱스 0, 1, 2, 3)
	AssignToQuickSlot(0, TEXT("HealItem"));
	AssignToQuickSlot(1, TEXT("BuffItem"));
	AssignToQuickSlot(2, TEXT("AttackItem"));
	AssignToQuickSlot(3, TEXT("ShieldItem"));
    
	UE_LOG(LogTemp, Warning, TEXT("퀵슬롯 세팅 완료!"));
}

void UInventoryManager::AddItem(FName ItemId, int32 Count)
{
	FInventoryItemSaveData* ExistingItem = InventoryItems.FindByPredicate([&](const FInventoryItemSaveData& Item) {
		return Item.ItemId == ItemId;
	});

	if (ExistingItem)
	{
		ExistingItem->Count += Count;
	}
	else 
	{ 
		FInventoryItemSaveData NewItem;
		NewItem.ItemId = ItemId;
		NewItem.Count = Count;
		InventoryItems.Add(NewItem); 
	}

	UOneTwoShootGameInstance* GI = Cast<UOneTwoShootGameInstance>(GetOuter());

	if (GI)
	{
		GI->RunData.InventoryItems = InventoryItems;
		GI->SaveProgress();
        
		if (GI->SaveProgress())
		{
			UE_LOG(LogTemp, Log, TEXT("아이템 추가 성공: %s, 저장 완료!"), *ItemId.ToString());
		}
	}
}

void UInventoryManager::UseItem(FName ItemId, ABaseTank* UserTank)
{
	// 1. GameInstance에서 해당 아이템 클래스(Blueprint)를 찾음
    // 2. 클래스 정보를 바탕으로 아이템 객체 생성 (NewObject)
    // 3. 생성된 아이템의 UseItem 함수 호출 (다형성 활용)
    
    // 이 구조로 구현하면 아이템마다 고유한 효과를 블루프린트에서 제어 가능함
	UE_LOG(LogTemp, Log, TEXT("성공! 아이템 사용됨: %s"), *ItemId.ToString());
}

void UInventoryManager::UseItemAtSlot(int32 SlotIndex, AActor* UserActor)
{
	ABaseTank* Tank = Cast<ABaseTank>(UserActor);
    
	if (Tank && InventoryItems.IsValidIndex(SlotIndex))
	{
		FName ItemId = InventoryItems[SlotIndex].ItemId;
		UseItem(ItemId, Tank); 
	}
}

// 퀵슬롯에 아이템 할당
void UInventoryManager::AssignToQuickSlot(int32 SlotIndex, FName ItemId)
{
	if (QuickSlotItems.IsValidIndex(SlotIndex))
	{
		QuickSlotItems[SlotIndex] = ItemId;
	}
}

// 퀵슬롯 아이템 사용
void UInventoryManager::UseQuickSlotItem(int32 SlotIndex, ABaseTank* UserTank)
{
	// 퀵슬롯 범위 확인
	if (QuickSlotItems.IsValidIndex(SlotIndex))
	{
		FName ItemId = QuickSlotItems[SlotIndex];
        
		// 아이템이 등록되어 있는지 확인
		if (ItemId != NAME_None)
		{
			// 사용 로직 호출
			UseItem(ItemId, UserTank); 
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("퀵슬롯 %d번이 비어있습니다!"), SlotIndex);
		}
	}
}