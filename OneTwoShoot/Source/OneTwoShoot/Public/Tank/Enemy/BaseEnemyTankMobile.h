// 움직일 수 있는 적 AI Base 클래스
#pragma once

#include "CoreMinimal.h"
#include "BaseEnemyTank.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "BaseEnemyTankMobile.generated.h"

// 전방 선언
class AVoxelWorld;

UCLASS()
class ONETWOSHOOT_API ABaseEnemyTankMobile : public ABaseEnemyTank
{
    GENERATED_BODY()

public:
    ABaseEnemyTankMobile();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    TArray<FIntVector> CurrentRemainingPath;

    virtual void MoveOnVoxelGrid() override;
    virtual void ExecuteVoxelMovement(TArray<FIntVector> Path);

    virtual void OnTurnStart() override;

    void OnMoveComplete(FAIRequestID RequestID, const FPathFollowingResult& Result);

    AVoxelWorld* GetVoxelWorld();

    UPROPERTY(BlueprintReadOnly, Category = "Voxel")
    TObjectPtr<class AVoxelWorld> VWorld;

    virtual void Fire();
};