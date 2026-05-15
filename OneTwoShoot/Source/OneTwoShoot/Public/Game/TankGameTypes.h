#pragma once

#include "CoreMinimal.h"
#include "TankGameTypes.generated.h"

class ABaseEnemyTank;

UENUM(BlueprintType)
enum class ETankPhase : uint8
{
	Wait		UMETA(DisplayName = "대기 (내 턴 아님)"),
	Move		UMETA(DisplayName = "이동 및 점프 단계"),
	Aim			UMETA(DisplayName = "조준 단계"),
	Action		UMETA(DisplayName = "발사 및 아이템 사용 단계")
};

UENUM(BlueprintType)
enum class EReloadSpeed : uint8
{
	Slow        UMETA(DisplayName = "느림"),
	Normal      UMETA(DisplayName = "보통"),
	Fast        UMETA(DisplayName = "빠름"),
	VeryFast    UMETA(DisplayName = "매우 빠름")
};

UENUM(BlueprintType)
enum class ETurnState : uint8
{
	Wait		UMETA(DisplayName = "대기 및 처리 중"),
	PlayerTurn	UMETA(DisplayName = "플레이어 차례"),
	EnemyTurn	UMETA(DisplayName = "적 차례")
};

UENUM(BlueprintType)
enum class ETankUnitType : uint8
{
	None,
	Player,
	Enemy
};

USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
	GENERATED_BODY()

	// 스폰할 적 탱크 클래스 (블루프린트 클래스 선택 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TSubclassOf<ABaseEnemyTank> EnemyClass;

	// 복쉘 월드 기준 스폰 격자 좌표 (X, Y)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	FIntPoint SpawnVoxelXY;
};

USTRUCT(BlueprintType)
struct FWaveData
{
	GENERATED_BODY()

	// 한 웨이브에 스폰할 적들의 정보 배열
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<FEnemySpawnInfo> EnemiesToSpawn;
};