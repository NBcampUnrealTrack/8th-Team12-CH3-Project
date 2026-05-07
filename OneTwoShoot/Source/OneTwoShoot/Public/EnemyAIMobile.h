// 움직일 수 있는 적 AI Base 클래스
#pragma once

#include "CoreMinimal.h"
#include "EnemyAIBase.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnemyAIMobile.generated.h"

// 전방 선언
class AVoxelWorld;

UCLASS()
class ONETWOSHOOT_API AEnemyAIMobile : public AEnemyAIBase
{
    GENERATED_BODY()

public:
    AEnemyAIMobile();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
    int32 MoveRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    TArray<FIntVector> CurrentRemainingPath;

    virtual void MoveOnVoxelGrid();
    virtual void ExecuteVoxelMovement(TArray<FIntVector> Path);
    virtual void DecideAction();

    virtual void OnTurnStart() override;

    void OnMoveComplete(FAIRequestID RequestID, const FPathFollowingResult& Result);

    AVoxelWorld* GetVoxelWorld();
};