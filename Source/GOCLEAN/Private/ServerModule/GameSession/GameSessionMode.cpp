// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerModule/GameSession/GameSessionMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"

#include "ServerModule/GameSession/GameSessionState.h"
#include "ServerModule/GameSession/PlayerSessionState.h"
#include "Net/UnrealNetwork.h"
#include <ServerModule/GameSession/GameSessionInstance.h>

#include "GMapSystem/Server/GMapManager.h"
#include "GCharacter/StatsComponent/CharacterStatsComponent.h"
#include "GCharacter/GOCLEANPlayerController.h"
#include "TimerManager.h"

#include "Kismet/GameplayStatics.h"
#include "GTypes/GMapTypes.h"

AGameSessionMode::AGameSessionMode()
{
	GameStateClass = AGameSessionState::StaticClass();
	PlayerStateClass = APlayerSessionState::StaticClass();
}


int32 AGameSessionMode::GetCurrentPlayerCount() const
{
    const AGameStateBase* GS = GameState;
    return GS ? GS->PlayerArray.Num() : 0;
}

bool AGameSessionMode::IsValidSeatIndex(int32 SeatIndex) const
{
    return (SeatIndex >= 0 && SeatIndex < MaxPlayers);
}

int32 AGameSessionMode::FindNextAvailableSeatIndex() const
{
    const AGameStateBase* GS = GameState;
    if (!GS) return INDEX_NONE;

    // 현재 사용 중인 좌석
    TSet<int32> UsedSeats;
    UsedSeats.Reserve(MaxPlayers);

    for (APlayerState* PS : GS->PlayerArray)
    {
        const APlayerSessionState* PSS = Cast<APlayerSessionState>(PS);
        if (!PSS) continue;

        const int32 Seat = PSS->GetSeatIndex();
        if (IsValidSeatIndex(Seat))
        {
            UsedSeats.Add(Seat);
        }
    }

    // 0~MaxPlayers-1 중 빈 곳 반환
    for (int32 i = 0; i < MaxPlayers; ++i)
    {
        if (!UsedSeats.Contains(i))
        {
            return i;
        }
    }
    return INDEX_NONE;
}

void AGameSessionMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (!HasAuthority() || !NewPlayer) return;
    if (!GameState) return;

    // 최대 인원 제한
    const int32 CurrentPlayers = GetCurrentPlayerCount();
    if (CurrentPlayers > MaxPlayers)
    {
        const FText Reason = FText::FromString(TEXT("방이 가득 찼습니다."));
        NewPlayer->ClientReturnToMainMenuWithTextReason(Reason);
        NewPlayer->Destroy();
        return;
    }

    //  PlayerState 확보
    APlayerSessionState* PSS = Cast<APlayerSessionState>(NewPlayer->PlayerState);
    if (!PSS)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Lobby] PlayerState cast failed"));
        return;
    }

    // 빈 좌석 배정 (0~3)
    const int32 Seat = FindNextAvailableSeatIndex();
    if (Seat == INDEX_NONE)
    {
        const FText Reason = FText::FromString(TEXT("좌석 할당 실패 (방이 가득 찼습니다)."));
        NewPlayer->ClientReturnToMainMenuWithTextReason(Reason);
        NewPlayer->Destroy();
        return;
    }

    PSS->SetSeatIndex(Seat);

    // 호스트는 Seat 0 / ready true 
    const bool bIsHostSeat = (Seat == 0);
    PSS->SetReady(bIsHostSeat);
    // 테스트용
    PSS->SetReady(true);

    PSS->SetNickname(PSS->GetPlayerName());

    // 로비 유아이 뜨도록
    if (AGOCLEANPlayerController* PC = Cast<AGOCLEANPlayerController>(NewPlayer))
    {
        PC->Client_ShowLobbyUI(); // 서버에서 호출
    }
}

void AGameSessionMode::Logout(AController* Exiting)
{
    APlayerController* ExitingPC = Cast<APlayerController>(Exiting);
    APlayerSessionState* ExitingPSS = ExitingPC ? Cast<APlayerSessionState>(ExitingPC->PlayerState) : nullptr;

    const int32 LeavingSeat = ExitingPSS ? ExitingPSS->GetSeatIndex() : INDEX_NONE;

    // 호스트 여부
    const bool bIsHostLeaving = (ExitingPC && ExitingPC->IsLocalController());

    if (Exiting)
    {
        if (APlayerState* PS = Exiting->PlayerState)
        {
            HandlePlayerLeft(PS->GetPlayerId());
        }
    }


    Super::Logout(Exiting);

    AGameStateBase* GS = GameState;
    AGameSessionState* SessionState = GetGameState<AGameSessionState>();


    // Index 당기기
    if (GS && LeavingSeat != INDEX_NONE)
    {
        for (APlayerState* PS : GS->PlayerArray)
        {
            APlayerSessionState* PSS = Cast<APlayerSessionState>(PS);
            if (!PSS) continue;

            if (PSS->GetSeatIndex() > LeavingSeat) 
            {
                PSS->SetSeatIndex(PSS->GetSeatIndex() - 1); 
            }
        }
    }

    // 생존자 수 갱신 (인게임 중 클라 강제 종료 시)
    if (SessionState && GS)
    {
        int32 AliveCount = 0;
        for (APlayerState* PS : GS->PlayerArray)
        {
            APlayerSessionState* PSS = Cast<APlayerSessionState>(PS);
            if (!PSS) continue;

            const bool bEliminated = PSS->IsEliminated();
            if (!bEliminated)
            {
                AliveCount++;
            }
        }

        SessionState->SetAliveSurvivorCount(AliveCount);

    }



    // 호스트가 떠날 경우 남은 플레이어 종료
    if (bIsHostLeaving)
    {
        const FText Reason = FText::FromString(TEXT("호스트가 나가서 세션이 종료되었습니다. 메인 메뉴로 돌아갑니다."));

        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            if (!PC) continue;

            if (PC->IsLocalController()) continue;

            PC->ClientReturnToMainMenuWithTextReason(Reason);
        }
    }

    OnPlayerReadyChanged();

    // Online Subsystem Steam 세션 종료 로직 추가
}

void AGameSessionMode::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    auto* GI = GetGameInstance<UGameSessionInstance>();
    auto* GSS = GetGameState<AGameSessionState>();
    if (!GI || !GSS) return;

    if (GSS->GetSelectedContractId() == 0)
    {
        GSS->SetSelectedContractId(GI->GetPendingContractId());
    }

    FTimerHandle T;
    GetWorldTimerManager().SetTimer(T, this, &AGameSessionMode::EnsureAllPlayersPawn, 0.2f, false);
}

void AGameSessionMode::OnPlayerReadyChanged()
{
    if (!HasAuthority()) return;
    StartGameIfPossible();
}

void AGameSessionMode::OnPlayerLoadStateChanged()
{
    if (!HasAuthority()) return;
    TransitionToInGameIfPossible();
}


bool AGameSessionMode::IsHostPlayerState(const APlayerState* PS) const
{
    if (!PS) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        const APlayerController* PC = It->Get();
        if (!PC) continue;

        if (PC->IsLocalController())
        {
            return (PC->PlayerState == PS);
        }
    }

    return false;
}

bool AGameSessionMode::CanStartGame() const
{
    if (!GameState) return false;

    int32 ActivePlayerCount = 0;
    int32 NonHostCount = 0;
    int32 NonHostReadyCount = 0;

    for (APlayerState* PS : GameState->PlayerArray)
    {
        APlayerSessionState* PSS = Cast<APlayerSessionState>(PS);
        if (!PSS) continue;


        ActivePlayerCount++;

        const bool bIsHost = IsHostPlayerState(PSS);
        if (bIsHost)
        {
            // 호스트는 Ready 체크에서 제외 (항상 Ready로 간주)
            continue;
        }

        NonHostCount++;
        if (PSS->IsReady())
        {
            NonHostReadyCount++;
        }
    }

    // 디버깅
    return true;

    //// 최소 인원 체크(호스트 포함)
    //if (ActivePlayerCount < MinPlayersToStart)
    //    return false;

    //// 호스트 제외 나머지 플레이어 레디 체크
    //return (NonHostReadyCount == NonHostCount);
}

void AGameSessionMode::StartGameIfPossible()
{
    if(bGameStarted) return;

    if (CanStartGame())
    {
        bGameStarted = true;

        // 전원이 레디면 로딩 처리
        TransitionToLoadingPhase();

        UE_LOG(LogTemp, Log, TEXT("[GameSessionMode] StartGame triggered."));
    }
}


bool AGameSessionMode::AreAllPlayersLoaded() const
{
    if (!GameState) return false;

    int32 ActiveCount = 0;
    for (APlayerState* PS : GameState->PlayerArray)
    {
        const APlayerSessionState* PSS = Cast<APlayerSessionState>(PS);
        if (!PSS) continue;

        ActiveCount++;
        if (!PSS->IsLoadingComplete())
            return false;
    }

    return (ActiveCount >= MinPlayersToStart);
}


void AGameSessionMode::TransitionToLoadingPhase()
{
    if (!HasAuthority()) return;

    AGameSessionState* GSS = GetGameState<AGameSessionState>();
    if (!GSS) return;

    GSS->SetSessionPhase(ESessionPhase::Loading);

    // 시작 시점에 모두 Loading으로 세팅
    for (APlayerState* PS : GameState->PlayerArray)
    {
        if (APlayerSessionState* PSS = Cast<APlayerSessionState>(PS))
        {
            PSS->SetLoadState(EPlayerLoadState::Loading);
        }
    }

    // 여기서 ServerTravel(맵 이동)
}


void AGameSessionMode::TransitionToInGameIfPossible()
{
    if (!HasAuthority()) return;

    AGameSessionState* GSS = GetGameState<AGameSessionState>();
    if (!GSS) return;

    // 로딩 페이즈에서만 InGame 전환 허용
    if (GSS->GetSessionPhase() != ESessionPhase::Loading)
        return;

    if (!AreAllPlayersLoaded())
        return;

    GSS->SetSessionPhase(ESessionPhase::InGame);
    GSS->SetInGamePhase(EInGamePhase::Cleaning);
}

bool AGameSessionMode::RequestSetContractId(int32 NewContractId)
{
    if (!HasAuthority()) return false;

    AGameSessionState* GSS = GetGameState<AGameSessionState>();
    if (!GSS) return false;

    GSS->SetSelectedContractId(NewContractId);
    return true;
}

bool AGameSessionMode::RequestPurchaseVending(int32 BuyerPlayerId, int32 ItemId)
{
    if (!HasAuthority()) return false;

    AGameSessionState* GSS = GetGameState<AGameSessionState>();
    if (!GSS) return false;

    if (GSS->GetSessionPhase() != ESessionPhase::Lobby) return false;

    return GSS->TryAddPurchasedVending(BuyerPlayerId, ItemId);
}

void AGameSessionMode::HandlePlayerLeft(int32 PlayerId)
{
    if (!HasAuthority()) return;

    if (AGameSessionState* GSS = GetGameState<AGameSessionState>())
    {
        GSS->RemovePurchasedVending(PlayerId);
    }
}


AGameSessionState* AGameSessionMode::GetSessionState() const
{
    return GetWorld() ? GetWorld()->GetGameState<AGameSessionState>() : nullptr;
}

bool AGameSessionMode::StartExorcismPhase()
{
    if (!HasAuthority()) return false;

    AGameSessionState* SS = GetSessionState();
    if (!SS) return false;

    const EInGamePhase Cur = SS->GetInGamePhase();

    if (Cur != EInGamePhase::Cleaning && Cur != EInGamePhase::ExorcismInProgress)
        return false;

    SS->SetInGamePhase(EInGamePhase::ExorcismStart);

    // 퇴마 시작 시 진행도/타이머 초기화
    SS->SetExorcismProgress(0.f, 0.f, 100.f);
    SS->StartPostExorcismTimer(0.f);

    return true;
}

bool AGameSessionMode::TryEnterExorcismInProgress(/*AActor* ExorcismCircle*/)
{
    if (!HasAuthority()) return false;

    AGameSessionState* SS = GetSessionState();
    if (!SS) return false;

    if (SS->GetInGamePhase() != EInGamePhase::ExorcismStart)
        return false;

    // =========
    // 각 플레이어들이 퇴마진과 동일한 방에 존재하는지 체크
    // =========

    const bool bAllPlayersInSameRoom = true; // MapManager로 방 체크 붙이기

    if (!bAllPlayersInSameRoom)
        return false;

    SS->SetInGamePhase(EInGamePhase::ExorcismInProgress);
    return true;
}

bool AGameSessionMode::FinishExorcismSuccess(float PostExorcismCountdownSeconds)
{
    if (!HasAuthority()) return false;

    AGameSessionState* SS = GetSessionState();
    if (!SS) return false;

    if (SS->GetInGamePhase() != EInGamePhase::ExorcismInProgress)
        return false;

    SS->SetInGamePhase(EInGamePhase::ExorcismEnd);

    SS->SetExorcismProgress(100.f, 0.f, 100.f);

    // 성공 후 카운트다운 시작
    BP_StartPostExorcismEscapeCountdown(PostExorcismCountdownSeconds);

    return true;
}

bool AGameSessionMode::FinishExorcismFail()
{
    if (!HasAuthority()) return false;

    AGameSessionState* SS = GetSessionState();
    if (!SS) return false;

    if (SS->GetInGamePhase() != EInGamePhase::ExorcismInProgress)
        return false;

    //============
    // 퇴마진 위치 바꾸기
    // ===========

    SS->SetInGamePhase(EInGamePhase::ExorcismStart);

    // 진행도 리셋
    SS->SetExorcismProgress(0.f, 0.f, 100.f);
    SS->StartPostExorcismTimer(0.f);

    return true;
}

void AGameSessionMode::ContractSuccess()
{
    if (!HasAuthority()) return;

    AGameSessionState* SS = GetSessionState();
    if (!SS) return;

    SS->SetSessionPhase(ESessionPhase::Ending);
    BP_OnContractSuccess();
}

void AGameSessionMode::ContractFail()
{
    if (!HasAuthority()) return;

    BP_OnContractFail();
}

void AGameSessionMode::BP_StartPostExorcismEscapeCountdown(float DurationSeconds)
{
    if (!HasAuthority()) return;

    AGameSessionState* SessionState = GetGameState<AGameSessionState>();
    if (!SessionState) return;

    SessionState->StartPostExorcismTimer(DurationSeconds);

    GetWorldTimerManager().ClearTimer(PostExorcismCountdownTimerHandle);
    GetWorldTimerManager().SetTimer(
        PostExorcismCountdownTimerHandle,
        this,
        &AGameSessionMode::TickPostExorcismEscapeCountdown,
        1.0f,
        true
    );
}

void AGameSessionMode::TickPostExorcismEscapeCountdown()
{
    if (!HasAuthority())
    {
        GetWorldTimerManager().ClearTimer(PostExorcismCountdownTimerHandle);
        return;
    }

    AGameSessionState* SessionState = GetGameState<AGameSessionState>();
    if (!SessionState)
    {
        GetWorldTimerManager().ClearTimer(PostExorcismCountdownTimerHandle);
        return;
    }

    SessionState->AddPostExorcismTimeRemaining(-1.f);

    if (SessionState->BP_GetPostExorcismTimeRemaining() <= 0.f)
    {
        GetWorldTimerManager().ClearTimer(PostExorcismCountdownTimerHandle);
        FinishPostExorcismEscapeCountdown();
    }
}

void AGameSessionMode::FinishPostExorcismEscapeCountdown()
{
    if (!HasAuthority()) return;

    AGameSessionState* SessionState = GetGameState<AGameSessionState>();
    if (!SessionState) return;

    UGMapManager* MapManager = SessionState->GetMapManager();

    if (GameState)
    {
        for (APlayerState* PS : GameState->PlayerArray)
        {
            APlayerSessionState* PSS = Cast<APlayerSessionState>(PS);
            if (!PSS) continue;

            APawn* Pawn = SessionState->GetPawnBySeat(PSS->GetSeatIndex());
            if (!Pawn) continue;

            const bool bIsOutdoor = (MapManager) ? MapManager->IsActorInZoneType(Pawn, EGZoneType::E_Outdoor) : false;
            if (!bIsOutdoor)
            {
                if (UCharacterStatsComponent* Stats = Pawn->FindComponentByClass<UCharacterStatsComponent>())
                {
                    const int32 CurLife = Stats->GetCurrentLife();
                    if (CurLife > 0)
                    {
                        Stats->DecreaseLife(CurLife); // 0으로 바꾸기
                    }
                }
            }
        }
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (AGOCLEANPlayerController* PC = Cast<AGOCLEANPlayerController>(It->Get()))
        {
            PC->Client_ShowResultUI();
        }
    }
}


UClass* AGameSessionMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    const APlayerController* PC = Cast<APlayerController>(InController);
    const APlayerSessionState* PSS = PC ? Cast<APlayerSessionState>(PC->PlayerState) : nullptr;

    if (PSS && InGameSeatPawnClasses.IsValidIndex(PSS->GetSeatIndex()) && InGameSeatPawnClasses[PSS->GetSeatIndex()])
    {
        return InGameSeatPawnClasses[PSS->GetSeatIndex()];
    }

    return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AGameSessionMode::PostSeamlessTravel()
{
    Super::PostSeamlessTravel();
    UE_LOG(LogTemp, Warning, TEXT("[InGameGM] PostSeamlessTravel called. HasAuthority=%d"), HasAuthority());
    if (!HasAuthority()) return;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = It->Get())
        {
            if (APawn* OldPawn = PC->GetPawn())
            {
                PC->UnPossess();
                OldPawn->Destroy();
            }

            RestartPlayer(PC);
        }
    }
}

bool AGameSessionMode::IsInGameMap() const
{
    const FString MapName = GetWorld() ? GetWorld()->GetMapName() : TEXT("FirstPersonMap");
    return !MapName.Contains(TEXT("FirstPersonMap"));
}

void AGameSessionMode::EnsureAllPlayersPawn()
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC) continue;

        if (PC->GetPawn() == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("RestartPlayer for %s"), *GetNameSafe(PC));
            RestartPlayer(PC);
        }
    }
}