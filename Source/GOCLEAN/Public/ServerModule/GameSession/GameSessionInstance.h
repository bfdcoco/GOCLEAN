// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameSessionInstance.generated.h"

/**
 *
 */
UCLASS()
class GOCLEAN_API UGameSessionInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void OnStart() override;

private:
    // Load Map

    UFUNCTION()
    void HandlePostLoadMap(UWorld* LoadedWorld);


    // UI

    void TryShowLevelUI();
    FTimerHandle UITryTimerHandle;

private:
    TWeakObjectPtr<UWorld> PendingUIWorld;

    int32 UITryCount = 0;
    static constexpr int32 MaxUITryCount = 60;


    void ScheduleTryShowUI(UWorld* InWorld, float DelaySeconds);

    
public:
    // 의뢰 선택
    UFUNCTION(BlueprintCallable)
    void SetPendingContractId(int32 NewContractId) { PendingContractId = NewContractId; }

    UFUNCTION(BlueprintCallable)
    int32 GetPendingContractId() const { return PendingContractId; }

private:
    // 타이틀에서 선택한 의뢰
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    int32 PendingContractId = 0;

public:
    // 조인 코드 설정 (타이틀에서 사용)
    UFUNCTION(BlueprintCallable, Category = "Session")
    void SetPendingJoinCode(const FString& NewCode);

    // 조인 코드 가져오기
    UFUNCTION(BlueprintCallable, Category = "Session")
    const FString& GetPendingJoinCode() const;

private:
    // 세션 생성 전에 임시 저장되는 조인 코드
    UPROPERTY()
    FString PendingJoinCode;

public:
    // VOICE

    // 보이스 채널 참가 여부 
    UFUNCTION(BlueprintCallable)
    bool IsVoiceJoined() const { return bVoiceJoined; }

    // raw 에너지 (0~1), Vivox에서 측정된 값 
    UFUNCTION(BlueprintCallable)
    float GetVoiceEnergyRaw01() const { return VoiceEnergyRaw01; }

    // 보정된 보이스 레벨. AI 추적에 사용 
    UFUNCTION(BlueprintCallable)
    float GetVoiceLevelCorrected01() const { return VoiceLevelCorrected01; }

    // 마이크 음소거 여부
    UFUNCTION(BlueprintCallable)
    bool IsMicMuted() const { return bVoiceMicMuted; }

    void GetAllVoiceStimuli(TMap<int32, TPair<FVector, float>>& OutVoiceMap) const
    {
        OutVoiceMap = VoiceStimuliMap;
    }


    // Subsystem 용 보이스 상태/캐시 업데이트
    void Voice_UpdateCache(bool bJoined, float EnergyRaw01, float LevelCorrected01, bool bMicMuted, const TMap<int32, TPair<FVector, float>>& InStimuliMap);

private:

    // 채널 참가 여부
    bool bVoiceJoined = false;

    // raw 에너지
    float VoiceEnergyRaw01 = 0.f;
    // 보정된 에너지
    float VoiceLevelCorrected01 = 0.f;

    // 마이크 상태
    bool bVoiceMicMuted = false;

    // PlayerIndex -> (Location, CorrectedLevel)
    TMap<int32, TPair<FVector, float>> VoiceStimuliMap;



};
