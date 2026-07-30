// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerModule/Sound/AmbientZoneActor.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

AAmbientZoneActor::AAmbientZoneActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
    SetRootComponent(Trigger);

    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionObjectType(ECC_WorldStatic);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Trigger->SetGenerateOverlapEvents(true);

    Trigger->OnComponentBeginOverlap.AddDynamic(this, &AAmbientZoneActor::OnTriggerBegin);
    Trigger->OnComponentEndOverlap.AddDynamic(this, &AAmbientZoneActor::OnTriggerEnd);

    bReplicates = false;
}

void AAmbientZoneActor::OnTriggerBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn) return;

    // 로컬 플레이어만 처리
    if (!Pawn->IsLocallyControlled()) return;

    // 여기서 EnterZone(this) 호출
}

void AAmbientZoneActor::OnTriggerEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn) return;
    if (!Pawn->IsLocallyControlled()) return;

    // Mgr->LeaveZone(this);
}

USoundBase* AAmbientZoneActor::PickRandomSfx() const
{
    if (RandomSfxList.Num() <= 0) return nullptr;
    const int32 Idx = FMath::RandRange(0, RandomSfxList.Num() - 1);
    return RandomSfxList[Idx];
}