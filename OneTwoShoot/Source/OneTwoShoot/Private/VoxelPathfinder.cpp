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

TArray<FIntVector> VoxelPathfinder::FindPath(AVoxelWorld* World, FIntVector Start, FIntVector End, int32 MaxSearchRange, AActor* Requester)
{
    TArray<FIntVector> FullPath;

    // 시작점/종착점 검사 시에도 Requester를 넘겨주어야 '나 자신'을 무시
    if (!World->IsWalkable(Start, Requester, End)) {
        UE_LOG(LogTemp, Error, TEXT("Pathfinding 실패: 시작 위치가 Walkable이 아님! Coords: %s"), *Start.ToString());
        return FullPath;
    }
    if (!World->IsWalkable(End, Requester, End)) {
        UE_LOG(LogTemp, Error, TEXT("Pathfinding 실패: 목적지 위치가 Walkable이 아님! Coords: %s"), *End.ToString());
        return FullPath;
    }

    if (!World || Start == End) return FullPath;

    if (!World->IsWalkable(Start, Requester, End) || !World->IsWalkable(End, Requester, End)) return FullPath;

    // 데이터 구조 초기화
    TArray<FIntVector> OpenList;
    TSet<FIntVector> ClosedList;
    TMap<FIntVector, FIntVector> ParentMap;
    TMap<FIntVector, int32> GScore;
    TMap<FIntVector, int32> FScore;

    OpenList.Add(Start);
    GScore.Add(Start, 0);
    FScore.Add(Start, GetManhattanDistance(Start, End));
    ParentMap.Add(Start, Start);

    int32 Iterations = 0;
    const int32 SafetyLimit = 2000;

    while (OpenList.Num() > 0 && Iterations < SafetyLimit)
    {
        Iterations++;

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

        if (Current == End)
        {
            FIntVector Temp = End;
            while (Temp != Start)
            {
                FullPath.Insert(Temp, 0);
                Temp = ParentMap[Temp];
            }
            return FullPath;
        }

        OpenList.Remove(Current);
        ClosedList.Add(Current);

        for (const FIntVector& Neighbor : GetNeighbors(Current))
        {
            if (ClosedList.Contains(Neighbor) || !World->IsWalkable(Neighbor, Requester, End))
            {
                continue;
            }

            int32 TentativeGScore = GScore[Current] + 1;

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

    return FullPath;
}