// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerSessionState.generated.h"

// 플레이어 로딩 상태
UENUM(BlueprintType)
enum class EPlayerLoadState : uint8
{
    None,
    Loading,
    Loaded
};


/**
 * 
 */
UCLASS()
class GOCLEAN_API APlayerSessionState : public APlayerState
{
	GENERATED_BODY()

public:
    APlayerSessionState();

    // Getters (public) 
    int32 GetSeatIndex() const { return SeatIndex; }
    const FString& GetNickname() const { return Nickname; }
    bool IsReady() const { return bReady; }
    bool IsEliminated() const { return bIsEliminated; }


    // 로딩 상태 가져오기
    UFUNCTION(BlueprintCallable, Category = "Player|Load")
    EPlayerLoadState GetLoadState() const { return LoadState; }

    // 로딩 완료 여부
    UFUNCTION(BlueprintCallable, Category = "Player|Load")
    bool IsLoadingComplete() const { return LoadState == EPlayerLoadState::Loaded; }


    // Client -> Server Requests (RPC) 
    UFUNCTION(Server, Reliable)
    void Server_SetReady(bool bNewReady);

    // Client -> Server 로딩 상태 보고
    UFUNCTION(Server, Reliable)
    void Server_SetLoadState(EPlayerLoadState NewState);

    UFUNCTION(BlueprintCallable, Category = "Player|Ready")
    void RequestSetReady(bool bNewReady);

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


public:
    // Setters (server-only, protected) 
    // GameMode에서만 호출하게
    void SetSeatIndex(int32 NewSeatIndex);
    void SetNickname(const FString& NewNickname);
    void SetReady(bool bNewReady);
    void SetEliminated(bool bNewEliminated);

    // 서버에서 상태 변경
    void SetLoadState(EPlayerLoadState NewState);

protected:
    // OnRep 
    UFUNCTION()
    void OnRep_SeatIndex();

    UFUNCTION()
    void OnRep_Nickname();

    UFUNCTION()
    void OnRep_Ready();

    UFUNCTION()
    void OnRep_Eliminated();


protected:
    UFUNCTION()
    void OnRep_LoadState();

    UFUNCTION(BlueprintImplementableEvent)
    void BP_OnLoadStateChanged(EPlayerLoadState NewState);

    // 로딩 상태
    UPROPERTY(ReplicatedUsing = OnRep_LoadState, BlueprintReadOnly)
    EPlayerLoadState LoadState = EPlayerLoadState::None;

private:
    // 세션 참여 순서
    UPROPERTY(ReplicatedUsing = OnRep_SeatIndex, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    int32 SeatIndex = -1;

    // 유저 닉네임 (스팀 닉네임)
    UPROPERTY(ReplicatedUsing = OnRep_Nickname, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    FString Nickname;

    // 준비 상태
    UPROPERTY(ReplicatedUsing = OnRep_Ready, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    bool bReady = false;

    // 탈락 상태 (true면 관전 상태)
    UPROPERTY(ReplicatedUsing = OnRep_Eliminated, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    bool bIsEliminated = false;

    // 후에 커스터마이징 정보 추가
	
};
