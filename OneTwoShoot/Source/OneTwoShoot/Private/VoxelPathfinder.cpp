// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/World/VoxelPathfinder.h"
#include "../Public/World/VoxelWorld.h"

VoxelPathfinder::VoxelPathfinder()
{
}

VoxelPathfinder::~VoxelPathfinder()
{
}

TArray<FIntVector> VoxelPathfinder::GetNeighbors(FIntVector Current)
{
    TArray<FIntVector> Neighbors;

    static const FIntVector HorizontalOffsets[] = {
        FIntVector(1, 0, 0),  // Forward
        FIntVector(-1, 0, 0), // Backward
        FIntVector(0, 1, 0),  // Right
        FIntVector(0, -1, 0)  // Left
    };

    // 수직 2방향 오프셋 (추후 개발을 위해 준비)
    /* static const FIntVector VerticalOffsets[] = {
        FIntVector(0, 0, 1),  // Up
        FIntVector(0, 0, -1)  // Down
    };
    */

    // 수평 이웃 추가
    for (const FIntVector& Offset : HorizontalOffsets)
    {
        Neighbors.Add(Current + Offset);
    }

    // 수직 2방향 오프셋 (추후 개발을 위해 준비), VerticalOffsets를 추가하면 됨.
    /*
    for (const FIntVector& Offset : VerticalOffsets)
    {
        Neighbors.Add(Current + Offset);
    }
    */

    return Neighbors;
}

int32 VoxelPathfinder::GetManhattanDistance(FIntVector A, FIntVector B)
{
    return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y) + FMath::Abs(A.Z - B.Z);
}

TArray<FIntVector> VoxelPathfinder::FindPath(AVoxelWorld* World, FIntVector Start, FIntVector End, int32 MaxSearchRange)
{
    TArray<FIntVector> FullPath;

    if (!World->IsWalkable(Start)) {
        UE_LOG(LogTemp, Error, TEXT("Pathfinding 실패: 보스 위치가 Walkable이 아님! Coords: %s"), *Start.ToString());
        return FullPath;
    }
    if (!World->IsWalkable(End)) {
        UE_LOG(LogTemp, Error, TEXT("Pathfinding 실패: 플레이어 위치가 Walkable이 아님! Coords: %s"), *End.ToString());
        return FullPath;
    }

    // 기초 예외 처리
    if (!World || Start == End) return FullPath;
    if (!World->IsWalkable(Start) || !World->IsWalkable(End))
    {
        UE_LOG(LogTemp, Warning, TEXT("VoxelPathfinder: 시작점 또는 끝점이 이동 불가능한 위치입니다."));
        return FullPath;
    }

    // A* 알고리즘을 위한 데이터 구조
    TArray<FIntVector> OpenList;             // 탐색해야 할 노드들
    TSet<FIntVector> ClosedList;             // 탐색이 완료된 노드들
    TMap<FIntVector, FIntVector> ParentMap;  // 경로 역추적용 (자식 -> 부모)
    TMap<FIntVector, int32> GScore;          // 시작점부터 현재까지의 실제 비용
    TMap<FIntVector, int32> FScore;          // 시작점 -> 현재 -> 끝점까지의 총 예상 비용 (G + H)

    // 시작점 초기화
    OpenList.Add(Start);
    GScore.Add(Start, 0);
    FScore.Add(Start, GetManhattanDistance(Start, End));
    ParentMap.Add(Start, Start);

    int32 Iterations = 0;
    const int32 SafetyLimit = 2000; // 무한 루프 방지를 위한 안전 장치

    while (OpenList.Num() > 0 && Iterations < SafetyLimit)
    {
        Iterations++;

        // 1. OpenList에서 FScore가 가장 낮은 노드를 현재 노드로 선택
        FIntVector Current = OpenList[0];
        int32 LowestF = FScore.Contains(Current) ? FScore[Current] : INT_MAX;

        for (const FIntVector& Node : OpenList)
        {
            int32 NodeF = FScore.Contains(Node) ? FScore[Node] : INT_MAX;
            if (NodeF < LowestF)
            {
                LowestF = NodeF;
                Current = Node;
            }
        }

        // 2. 목적지 도달 확인
        if (Current == End)
        {
            // 경로 역추적하여 FullPath 생성
            FIntVector Temp = End;
            while (Temp != Start)
            {
                FullPath.Insert(Temp, 0); // 배열 맨 앞에 추가하여 순서 맞춤
                Temp = ParentMap[Temp];
            }
            return FullPath;
        }

        // 3. 현재 노드를 OpenList에서 빼고 ClosedList에 추가
        OpenList.Remove(Current);
        ClosedList.Add(Current);

        // 4. 이웃 노드 탐색 (GetNeighbors 호출)
        for (const FIntVector& Neighbor : GetNeighbors(Current))
        {
            // 이미 탐색했거나 이동 불가능한 칸이면 건너뛰기
            if (ClosedList.Contains(Neighbor) || !World->IsWalkable(Neighbor))
            {
                continue;
            }

            // 새로운 GScore 계산 (현재까지의 거리 + 1칸)
            int32 TentativeGScore = GScore[Current] + 1;

            // 아직 발견하지 못한 노드이거나, 더 짧은 경로를 찾은 경우
            if (!GScore.Contains(Neighbor) || TentativeGScore < GScore[Neighbor])
            {
                ParentMap.FindOrAdd(Neighbor) = Current;
                GScore.FindOrAdd(Neighbor) = TentativeGScore;
                FScore.FindOrAdd(Neighbor) = TentativeGScore + GetManhattanDistance(Neighbor, End);

                if (!OpenList.Contains(Neighbor))
                {
                    OpenList.Add(Neighbor);
                }
            }
        }
    }

    if (Iterations >= SafetyLimit)
    {
        UE_LOG(LogTemp, Error, TEXT("VoxelPathfinder: 탐색 한도 초과로 경로 찾기 실패!"));
    }

    return FullPath; // 경로를 못 찾은 경우 빈 배열 반환
}