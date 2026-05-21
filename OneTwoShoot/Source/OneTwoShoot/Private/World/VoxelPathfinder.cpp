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
        FIntVector(0, -1, 0),  // Left

        FIntVector(1, 1, 0),  // Forward-Right
        FIntVector(1, -1, 0), // Forward-Left
        FIntVector(-1, 1, 0), // Backward-Right
        FIntVector(-1, -1, 0) // 좌하단 (Backward-Left)
    };

    for (const FIntVector& Offset : HorizontalOffsets)
    {
        Neighbors.Add(Current + Offset);
    }

    return Neighbors;
}

int32 VoxelPathfinder::GetManhattanDistance(FIntVector A, FIntVector B)
{
    int32 dX = FMath::Abs(A.X - B.X);
    int32 dY = FMath::Abs(A.Y - B.Y);
    int32 dZ = FMath::Abs(A.Z - B.Z);

    return FMath::Max(dX, dY) + dZ;
}

TArray<FIntVector> VoxelPathfinder::FindPath(AVoxelWorld* World, FIntVector Start, FIntVector End, int32 MaxSearchRange, AActor* Requester)
{
    TArray<FIntVector> FullPath;

    // 시작점/종착점 검사 시에도 Requester를 넘겨주어야 '나 자신'을 무시
    if (!World->IsWalkable(Start, Requester, End)) {
        UE_LOG(LogTemp, Error, TEXT("Pathfinding 실패: 시작 위치(%s)가 Walkable이 아님! (탱크가 땅에 파묻힘?)"), *Start.ToString());
        return FullPath;
    }
    if (!World->IsWalkable(End, Requester, End)) {
        UE_LOG(LogTemp, Error, TEXT("Pathfinding 실패: 목적지 위치(%s)가 Walkable이 아님! (플레이어가 허공/땅속에 있음?)"), *End.ToString());
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
    FScore.Add(Start, GetManhattanDistance(Start, End) * 10); // 정수 연산을 위해 가중치 배율 통일
    ParentMap.Add(Start, Start);

    int32 Iterations = 0;
    const int32 SafetyLimit = 5000;

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

        for (const FIntVector& NeighborNode : GetNeighbors(Current)) // GetNeighbors는 수평 4방향만 반환
        {
            FIntVector Neighbor = NeighborNode;
            FIntVector ActualNeighbor = Neighbor;

            // 1. 평면(동일 Z) 이동이 가능한가?
            if (World->IsWalkable(Neighbor, Requester, End))
            {
                ActualNeighbor = Neighbor;
            }
            // 2. 평면이 막혀있다면, 한 칸 오르막(Z + 1)은 가능한가?
            else if (World->IsWalkable(Neighbor + FIntVector(0, 0, 1), Requester, End))
            {
                ActualNeighbor = Neighbor + FIntVector(0, 0, 1);
            }
            // 3. 평면이 허공이라면, 한 칸 내리막(Z - 1)은 가능한가?
            else if (World->IsWalkable(Neighbor + FIntVector(0, 0, -1), Requester, End))
            {
                ActualNeighbor = Neighbor + FIntVector(0, 0, -1);
            }
            // 3가지 모두 불가능하면 진짜 벽(장애물)이므로 스킵
            else
            {
                continue;
            }

            if (ClosedList.Contains(ActualNeighbor)) continue;

            //직진은 10점, 대각선은 14점(1.4배)을 부여하여 정수 기반의 정밀한 최단 거리를 유도.
            int32 TentativeGScore = (ActualNeighbor.X != Current.X && ActualNeighbor.Y != Current.Y) ? 14 : 10;

            if (!GScore.Contains(ActualNeighbor) || TentativeGScore < GScore[ActualNeighbor])
            {
                ParentMap.FindOrAdd(ActualNeighbor) = Current;
                GScore.FindOrAdd(ActualNeighbor) = TentativeGScore;
                FScore.FindOrAdd(ActualNeighbor) = TentativeGScore + GetManhattanDistance(ActualNeighbor, End);

                if (!OpenList.Contains(ActualNeighbor))
                {
                    OpenList.Add(ActualNeighbor);
                }
            }
        }

    }

    UE_LOG(LogTemp, Error, TEXT("Pathfinding 실패: A* 루프가 끝났으나 타겟(%s)에 닿을 수 없음! (지형이 막혀있거나 단절됨)"), *End.ToString());
    return FullPath;
}