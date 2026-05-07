#include "../Public/Tank/Enemy/EnemyAIMobile.h"
#include "../Public/World/VoxelPathfinder.h"
#include "../Public/World/VoxelWorld.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyAIMobile::AEnemyAIMobile()
{
    MoveRange = 3;
    bIsMoving = false;

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
}

void AEnemyAIMobile::BeginPlay()
{
    Super::BeginPlay();

    if (!TargetPlayer)
    {
        TargetPlayer = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

        if (TargetPlayer)
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] 타겟 플레이어(%s)를 성공적으로 찾았습니다."),
                *GetName(), *TargetPlayer->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[%s] 플레이어 Pawn을 찾았으나 ACharacter 타입이 아닙니다!"), *GetName());
        }
    }
}

void AEnemyAIMobile::OnTurnStart()
{
    Super::OnTurnStart();

    UE_LOG(LogTemp, Warning, TEXT("[%s] Mobile OnTurnStart 호출됨"), *GetName());
    DecideAction();
}

void AEnemyAIMobile::MoveOnVoxelGrid()
{
    // 1. 만약 TargetPlayer가 비어있다면, 지금 즉시 다시 찾아봅니다.
    if (!TargetPlayer)
    {
        TargetPlayer = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

        if (TargetPlayer) {
            UE_LOG(LogTemp, Warning, TEXT("[%s] 이동 직전에 플레이어를 찾았습니다!"), *GetName());
        }
    }

    AVoxelWorld* VWorld = GetVoxelWorld();

    // 2. VWorld도 마찬가지로 못 찾았다면 지금 다시 찾습니다.
    if (!VWorld) {
        VWorld = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass()));
    }

    // --- 검사 로직 ---
    if (!VWorld) {
        UE_LOG(LogTemp, Error, TEXT("[%s] VWorld를 찾을 수 없습니다!"), *GetName());
        return;
    }

    if (!TargetPlayer) {
        // 여기서 로그가 찍힌다면 진짜로 월드에 플레이어가 없는 것입니다.
        UE_LOG(LogTemp, Error, TEXT("[%s] 월드에 플레이어 캐릭터가 존재하지 않습니다!"), *GetName());
        return;
    }

    // --- 여기서부터는 안전하게 경로 탐색 시작 ---
    UE_LOG(LogTemp, Warning, TEXT("[%s] 경로 탐색을 시작합니다."), *GetName());

    FIntVector Start = VWorld->WorldToVoxelCoords(GetActorLocation());
    FIntVector End = VWorld->WorldToVoxelCoords(TargetPlayer->GetActorLocation());

    // 3. 좌표 유효성 체크 (IsWalkable 로그 확인)
    UE_LOG(LogTemp, Warning, TEXT("DEBUG: 탐색 시작점: %s (Walkable: %s)"),
        *Start.ToString(), VWorld->IsWalkable(Start) ? TEXT("True") : TEXT("False"));
    UE_LOG(LogTemp, Warning, TEXT("DEBUG: 탐색 종착점: %s (Walkable: %s)"),
        *End.ToString(), VWorld->IsWalkable(End) ? TEXT("True") : TEXT("False"));

    // 4. 경로 탐색 실행
    TArray<FIntVector> FullPath = VoxelPathfinder::FindPath(VWorld, Start, End, 100);

    if (FullPath.Num() == 0) {
        UE_LOG(LogTemp, Error, TEXT("DEBUG: 경로 탐색 결과 0칸! (길이 막혔거나 범위 초과)"));
        OnMoveComplete(FAIRequestID(), FPathFollowingResult(EPathFollowingResult::Aborted));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("DEBUG: 경로 탐색 성공! 총 %d칸 중 %d칸 이동 실행"),
        FullPath.Num(), FMath::Min(FullPath.Num() - 1, MoveRange));

    // 이동 실행
    ExecuteVoxelMovement(FullPath);
}

void AEnemyAIMobile::ExecuteVoxelMovement(TArray<FIntVector> Path)
{
    if (Path.Num() == 0)
    {
        // 모든 예정된 칸 이동 완료!
        bIsMoving = false;
        DecideAction();
        return;
    }

    bIsMoving = true;

    // 다음 목적지 좌표 추출 및 배열에서 제거
    FIntVector NextStepCoords = Path[0];
    Path.RemoveAt(0);

    // 복쉘 좌표를 실제 월드 좌표로 변환
    AVoxelWorld* VWorld = GetVoxelWorld();
    FVector TargetWorldPos = VWorld->VoxelToWorldLocation(NextStepCoords);

    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);

        // 한 칸 이동 명령 (도착 시 OnMoveComplete를 호출하게 됨)
        // 이때 남은 경로(Path)를 안전하게 전달하기 위해 람다(Lambda)나 멤버 변수를 활용할 수 있습니다.
        // 여기서는 가장 직관적으로 OnMoveComplete가 남은 Path를 가지고 재귀 호출하도록 연결합니다.

        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(TargetWorldPos);
        MoveRequest.SetAcceptanceRadius(5.0f); // 복쉘 중심에 정지

        AIC->MoveTo(MoveRequest);

        AIC->GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &AEnemyAIMobile::OnMoveComplete);

        CurrentRemainingPath = Path;
    }
}

void AEnemyAIMobile::OnMoveComplete(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] 이동 완료 - 성공 여부: %s"),
        *GetName(), Result.IsSuccess() ? TEXT("성공") : TEXT("실패"));

    if (Result.IsSuccess())
    {
        ExecuteVoxelMovement(CurrentRemainingPath);
    }
    else
    {
        // 이동 실패(장애물 등) 시 행동 종료
        UE_LOG(LogTemp, Error, TEXT("[%s] 복쉘 이동 중단됨."), *GetName());
        bIsMoving = false;
        OnTurnEnd();
    }
}

void AEnemyAIMobile::DecideAction()
{
    // 하위 클래스에서 구현
    // ex) 자폭형 → 근접 여부 확인 후 폭발
    //     이동형 포격 → Aim() → Fire()
}

AVoxelWorld* AEnemyAIMobile::GetVoxelWorld()
{
    AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass());
    return Cast<AVoxelWorld>(FoundActor);
}


//void AEnemyAIMobile::Move()
//{
//    if (!TargetPlayer) return;
//
//    AAIController* AIC = Cast<AAIController>(GetController());
//    if (!AIC) return;
//
//    // 바인딩 전에 항상 먼저 해제 (중복 방지)
//    AIC->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
//
//    // 플레이어 방향으로 MoveRange만큼 이동
//    FVector Direction = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
//    FVector TargetLocation = GetActorLocation() + Direction * MoveRange;
//
//    bIsMoving = true;
//
//    // 이동 완료 시 OnMoveComplete 콜백
//    FAIMoveRequest MoveRequest;
//    MoveRequest.SetGoalLocation(TargetLocation);
//    MoveRequest.SetAcceptanceRadius(10.f);
//
//    FNavPathSharedPtr NavPath;
//    AIC->MoveTo(MoveRequest, &NavPath);
//
//    // 이동 완료 델리게이트 바인딩
//    AIC->GetPathFollowingComponent()->OnRequestFinished.AddUObject(
//        this, &AEnemyAIMobile::OnMoveComplete
//    );
//
//
//    UE_LOG(LogTemp, Warning, TEXT("[%s] 이동 시작 - 목표 위치: %s"),
//        *GetName(), *TargetLocation.ToString());
//}