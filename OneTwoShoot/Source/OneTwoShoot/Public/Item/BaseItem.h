#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BaseItem.generated.h"

class ABaseTank;
class UTexture2D;

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None		UMETA(DisplayName = "없음"),
	Projectile	UMETA(DisplayName = "투사체형"),
	Utility		UMETA(DisplayName = "유틸형"),
	Passive		UMETA(DisplayName = "패시브형")
};

UCLASS(Abstract, Blueprintable, BlueprintType)
class ONETWOSHOOT_API UBaseItem : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	FName ItemName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	UTexture2D* ItemIcon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	FText ItemDescription;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	EItemType ItemType;
	
	UFUNCTION(BlueprintCallable, Category = "Item Action")
	virtual void UseItem(ABaseTank* User);
	
	UFUNCTION(BlueprintCallable, Category = "Item Action")
	virtual bool ConsumOnUse() const;
};
