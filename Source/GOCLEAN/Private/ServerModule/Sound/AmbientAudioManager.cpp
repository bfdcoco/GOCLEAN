// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerModule/Sound/AmbientAudioManager.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

#include "GMapSystem/Server/GMapManager.h"
#include "GTypes/GMapTypes.h"


void UAmbientAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UAmbientAudioManager::Deinitialize()
{
    StopAmbient();
    Super::Deinitialize();
}

UWorld* UAmbientAudioManager::GetWorldSafe() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return World;
}

UGMapManager* UAmbientAudioManager::GetMapManager() const
{
    UWorld* World = GetWorldSafe();
    if (!World) return nullptr;

    return World->GetSubsystem<UGMapManager>();
}

//void UAmbientAudioManager::StartAmbient()
//{
//    if (bRunning) return;
//
//    bRunning = true;
//    bIsOutdoor = false;
//    CurrentZoneId = NAME_None;
//
//    UWorld* World = GetWorldSafe();
//    if (!World) return;
//
//    // 컨텍스트 폴링 시작
//    World->GetTimerManager().ClearTimer(ContextPollTimer);
//    World->GetTimerManager().SetTimer(
//        ContextPollTimer,
//        this,
//        &UAmbientAudioManager::TickContext,
//        ContextPollInterval,
//        true
//    );
//
//    // 즉시 1회 갱신
//    TickContext();
//}

void UAmbientAudioManager::StopAmbient()
{
    bRunning = false;

    if (UWorld* World = GetWorldSafe())
    {
        World->GetTimerManager().ClearTimer(ContextPollTimer);
        World->GetTimerManager().ClearTimer(EventSfxTimer);
    }

    // 2D 루프 정지
    if (BaseLoopComp)
    {
        BaseLoopComp->Stop();
        BaseLoopComp = nullptr;
    }

    bIsOutdoor = false;
    CurrentZoneId = NAME_None;
}

void UAmbientAudioManager::StartAmbient(USoundBase* InIndoorLoop, USoundBase* InOutdoorLoop, UDataTable* InZoneTable)
{
    IndoorBaseLoop = InIndoorLoop;
    OutdoorBaseLoop = InOutdoorLoop;
    ZoneAudioTable = InZoneTable;

    if (bRunning) return;

    bRunning = true;
    bIsOutdoor = false;
    CurrentZoneId = NAME_None;

    UWorld* World = GetWorldSafe();
    if (!World) return;

    // 컨텍스트 폴링 시작
    World->GetTimerManager().ClearTimer(ContextPollTimer);
    World->GetTimerManager().SetTimer(
        ContextPollTimer,
        this,
        &UAmbientAudioManager::TickContext,
        ContextPollInterval,
        true
    );

    // 즉시 1회 갱신
    TickContext();
}

void UAmbientAudioManager::TickContext()
{
    if (!bRunning) return;

    UWorld* World = GetWorldSafe();
    if (!World) return;

    // 로컬 플레이어 Pawn 기준 (멀티에서 각 클라가 자기 것만 실행)
    APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!Pawn) return;

    UGMapManager* MapMgr = GetMapManager();
    if (!MapMgr) return;

    // 1) 실내/실외 판정 → 베이스 루프 갱신
    const bool bNowOutdoor = MapMgr->IsActorInZoneType(Pawn, EGZoneType::E_Outdoor);
    if (bNowOutdoor != bIsOutdoor)
    {
        bIsOutdoor = bNowOutdoor;
        UpdateBaseLoop(bIsOutdoor);

        if (bDebugLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Ambient] BaseLoop switched: %s"),
                bIsOutdoor ? TEXT("Outdoor") : TEXT("Indoor"));
        }
    }

    // 2) 현재 ZoneID 목록 → 대표 ZoneID 선택
    const TArray<FName> ZoneIds = MapMgr->GetActorZoneIds(Pawn);
    const FName NewZoneId = ChooseDominantZoneId(ZoneIds, MapMgr);

    if (NewZoneId != CurrentZoneId)
    {
        CurrentZoneId = NewZoneId;

        if (bDebugLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Ambient] Zone changed: %s"),
                *CurrentZoneId.ToString());
        }

        // Zone이 바뀌면 이벤트 SFX 스케줄 재설정
        RescheduleEventSfx();
    }
}

void UAmbientAudioManager::UpdateBaseLoop(bool bNowOutdoor)
{
    UWorld* World = GetWorldSafe();
    if (!World) return;

    USoundBase* Target = bNowOutdoor ? OutdoorBaseLoop : IndoorBaseLoop;
    if (!Target)
    {
        // 타겟 루프가 없으면 끔
        if (BaseLoopComp)
        {
            BaseLoopComp->Stop();
            BaseLoopComp = nullptr;
        }
        return;
    }

    // 이미 같은 사운드가 재생 중이면 유지
    if (BaseLoopComp && BaseLoopComp->Sound == Target && BaseLoopComp->IsPlaying())
    {
        return;
    }

    // 기존 루프 정지
    if (BaseLoopComp)
    {
        BaseLoopComp->Stop();
        BaseLoopComp = nullptr;
    }

    // 새 루프 시작
    BaseLoopComp = UGameplayStatics::SpawnSound2D(World, Target, BaseLoopVolume);
    if (BaseLoopComp)
    {
        BaseLoopComp->bAutoDestroy = false;
    }
}

FName UAmbientAudioManager::ChooseDominantZoneId(const TArray<FName>& ZoneIds, UGMapManager* MapMgr) const
{
    if (!MapMgr) return NAME_None;
    if (ZoneIds.Num() == 0) return NAME_None;

    // 우선순위 룰: Room(3) > Basecamp(2) > Corridor(1) > Outdoor(0)
    int32 BestScore = INT_MIN;
    FName BestId = NAME_None;

    for (const FName& Id : ZoneIds)
    {
        const FGZoneData* Z = MapMgr->GetZoneData(Id);
        if (!Z) continue;

        int32 Score = 0;
        switch (Z->Type)
        {
        case EGZoneType::E_Room:     Score = 3; break;
        case EGZoneType::E_Basecamp: Score = 2; break;
        case EGZoneType::E_Corridor: Score = 1; break;
        case EGZoneType::E_Outdoor:  Score = 0; break;
        default:                     Score = 0; break;
        }

        if (Score > BestScore)
        {
            BestScore = Score;
            BestId = Id;
        }
    }

    return BestId;
}

const FAmbientZoneAudioRow* UAmbientAudioManager::FindZoneAudioRow(FName ZoneId) const
{
    if (!ZoneAudioTable) return nullptr;
    if (ZoneId.IsNone()) return nullptr;

    // RowName을 ZoneId로 맞춰 쓰는 방식
    return ZoneAudioTable->FindRow<FAmbientZoneAudioRow>(ZoneId, TEXT("AmbientZoneAudioRow"));
}

USoundBase* UAmbientAudioManager::PickRandom(const TArray<TObjectPtr<USoundBase>>& List)
{
    if (List.Num() <= 0) return nullptr;

    const int32 Idx = FMath::RandRange(0, List.Num() - 1);
    return List[Idx].Get();
}

void UAmbientAudioManager::RescheduleEventSfx()
{
    UWorld* World = GetWorldSafe();
    if (!World) return;

    World->GetTimerManager().ClearTimer(EventSfxTimer);

    if (!bEnableEventSfx) return;
    if (CurrentZoneId.IsNone()) return;

    const FAmbientZoneAudioRow* Row = FindZoneAudioRow(CurrentZoneId);
    if (!Row) return;
    if (Row->RandomSfxList.Num() == 0) return;

    const float MinT = FMath::Max(0.1f, Row->MinInterval);
    const float MaxT = FMath::Max(MinT, Row->MaxInterval);
    const float Delay = FMath::FRandRange(MinT, MaxT);

    World->GetTimerManager().SetTimer(
        EventSfxTimer,
        this,
        &UAmbientAudioManager::TryPlayEventSfx,
        Delay,
        false
    );
}

void UAmbientAudioManager::TryPlayEventSfx()
{
    UWorld* World = GetWorldSafe();
    if (!World) return;

    if (!bEnableEventSfx)
    {
        RescheduleEventSfx();
        return;
    }

    APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!Pawn)
    {
        RescheduleEventSfx();
        return;
    }

    const FAmbientZoneAudioRow* Row = FindZoneAudioRow(CurrentZoneId);
    if (!Row)
    {
        RescheduleEventSfx();
        return;
    }

    const float Chance = FMath::Clamp(Row->PlayChance, 0.f, 1.f);
    if (FMath::FRand() > Chance)
    {
        RescheduleEventSfx();
        return;
    }

    USoundBase* Sfx = PickRandom(Row->RandomSfxList);
    if (!Sfx)
    {
        RescheduleEventSfx();
        return;
    }

    FVector Loc = Pawn->GetActorLocation();

    if (Row->bPlayNearPlayer)
    {
        const float R = FMath::Max(0.f, Row->NearPlayerRadius);
        if (R > 0.f)
        {
            Loc += FMath::VRand() * FMath::FRandRange(0.f, R);
        }
    }

    // 3D 이벤트 사운드 재생
    UGameplayStatics::PlaySoundAtLocation(World, Sfx, Loc);

    if (bDebugLog)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ambient] EventSfx Played: Zone=%s"), *CurrentZoneId.ToString());
    }

    // 다음 예약
    RescheduleEventSfx();
}