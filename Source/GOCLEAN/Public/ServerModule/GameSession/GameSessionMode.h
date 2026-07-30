// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameSessionMode.generated.h"

/**
 * 
 */
UCLASS()
class GOCLEAN_API AGameSessionMode : public AGameModeBase
{
	GENERATED_BODY()

public:
    AGameSessionMode();

protected:
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual void BeginPlay() override;

public:
    // Ready가 바뀌었을 때 호출(서버)
    void OnPlayerReadyChanged();

    // 플레이어 로딩 완료 체크
    void OnPlayerLoadStateChanged();
protected:
    // 실제 게임 시작(맵 이동 등)
    void StartGameIfPossible();

    // 시작 가능 여부 판단
    bool CanStartGame() const;
    
    // 호스트 판별
    bool IsHostPlayerState(const APlayerState* PS) const;

    // 로딩 완료 체크/전환
    bool AreAllPlayersLoaded() const;
    void TransitionToLoadingPhase();
    void TransitionToInGameIfPossible();

public:
    // 로비에서 호스트가 의뢰(맵) 선택
    bool RequestSetContractId(int32 NewContractId);

    // 벤딩 구매 요청 (서버에서 규칙 검사 후 반영)
    bool RequestPurchaseVending(int32 BuyerPlayerId, int32 ItemId);

    // 플레이어 튕김/퇴장 시 구매 제거
    void HandlePlayerLeft(int32 PlayerId);

private:
    // 최대 동시 플레이어 수
    UPROPERTY(EditDefaultsOnly, Category = "Session")
    int32 MaxPlayers = 4;

    // 시작을 위한 최소 레디 인원
    UPROPERTY(EditDefaultsOnly, Category = "Session")
    int32 MinPlayersToStart = 2; 

    // 게임 시작이 가능한지
    bool bGameStarted = false;


protected:
    // 현재 접속(게임에 존재) 중인 플레이어 수: 2/3/4 
    UFUNCTION(BlueprintCallable, Category = "Lobby|Seat")
    int32 GetCurrentPlayerCount() const;

    // 0~MaxPlayers-1 범위에서 비어있는 좌석 인덱스 찾기. 꽉 차면 INDEX_NONE
    UFUNCTION(BlueprintCallable, Category = "Lobby|Seat")
    int32 FindNextAvailableSeatIndex() const;

    // SeatIndex가 유효 범위(0~MaxPlayers-1)인지
    UFUNCTION(BlueprintCallable, Category = "Lobby|Seat")
    bool IsValidSeatIndex(int32 SeatIndex) const;


public:
    // 청소 중(또는 퇴마 중) -> 퇴마 시작
    UFUNCTION(BlueprintCallable, Category = "Session|Exorcism")
    bool StartExorcismPhase();

    // 퇴마진 첫 인터랙션 시 호출 (조건 만족 시 퇴마 시작 -> 퇴마 중)
    UFUNCTION(BlueprintCallable, Category = "Session|Exorcism")
    bool TryEnterExorcismInProgress(/*AActor* ExorcismCircle*/);

    // 퇴마 성공 (퇴마 중 -> 퇴마 종료 + 카운트다운 시작)
    UFUNCTION(BlueprintCallable, Category = "Session|Exorcism")
    bool FinishExorcismSuccess(float PostExorcismCountdownSeconds = 10.f);

    // 퇴마 실패 (퇴마 중 -> 퇴마 시작 (퇴마진 위치 변경)
    UFUNCTION(BlueprintCallable, Category = "Session|Exorcism")
    bool FinishExorcismFail();

    // 의뢰 성공/실패 결과창 띄우기
    UFUNCTION(BlueprintCallable, Category = "Session|Contract")
    void ContractSuccess();

    UFUNCTION(BlueprintCallable, Category = "Session|Contract")
    void ContractFail();

protected:
    // BP에서 결과 UI를 띄우게 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Session|Contract")
    void BP_OnContractSuccess();

    UFUNCTION(BlueprintImplementableEvent, Category = "Session|Contract")
    void BP_OnContractFail();

private:
    class AGameSessionState* GetSessionState() const;

public:
    UFUNCTION(BlueprintCallable, Category = "InGame|Exorcism")
    void BP_StartPostExorcismEscapeCountdown(float DurationSeconds = 30.f);

    // countdown tick / finish
    void TickPostExorcismEscapeCountdown();
    void FinishPostExorcismEscapeCountdown();

private:
    FTimerHandle PostExorcismCountdownTimerHandle;


// 임시 커스터마이징

public:
    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

    virtual void PostSeamlessTravel() override; // SeamlessTravel 후 호출

protected:
    // 인게임에서만 실행하고 싶으면 true 반환하도록(맵 이름/플래그로 판단)
    bool IsInGameMap() const;

    void EnsureAllPlayersPawn();

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
    TArray<TSubclassOf<APawn>> InGameSeatPawnClasses;


};
