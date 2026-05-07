// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class AVoxelWorld;
enum class EVoxelBlockType : uint8;

struct FVoxelPathNode
{
	FIntVector Coords;      // 복쉘 좌표
	int32 GCost;            // 시작점으로부터의 거리
	int32 HCost;            // 목적지까지의 예상 거리
	FIntVector Parent;      // 경로 역추적을 위한 부모 좌표

	int32 GetFCost() const { return GCost + HCost; } // 계산

	bool operator==(const FVoxelPathNode& Other) const { return Coords == Other.Coords; }
};

class ONETWOSHOOT_API VoxelPathfinder
{
public:
	VoxelPathfinder();
	~VoxelPathfinder();

	static TArray<FIntVector> FindPath(AVoxelWorld* World, FIntVector Start, FIntVector End, int32 MaxRange);

private:
	// 맨해튼 거리 계산 ($|x1-x2| + |y1-y2| + |z1-z2|$)
	static int32 GetManhattanDistance(FIntVector A, FIntVector B);

	// 유효한 이웃 노드(6방향)를 가져옴
	static TArray<FIntVector> GetNeighbors(FIntVector Current);
};