#include "../Public/Item/BaseItem.h"
#include "../Public/Tank/BaseTank.h"

UBaseItem::UBaseItem()
{
	ItemName = TEXT("기본 아이템");
	ItemDescription = FText::FromString(TEXT("기본 아이템 설명입니다."));
	ItemType = EItemType::None;
	ItemIcon = nullptr;
}

void UBaseItem::UseItem(ABaseTank* User)
{
	if (User)
	{
		UE_LOG(LogTemp, Log, TEXT("%s 가 %s 아이템을 사용했습니다."), *User->GetName(), *ItemName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("아이템 사용 실패. 사용자가 유효하지 않습니다."));
	}
}

bool UBaseItem::ConsumeOnUse() const
{
	return true;
}