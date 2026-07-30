// Fill out your copyright notice in the Description page of Project Settings.

#include "GObjectSystem/ExocismCircle.h"

#include "Net/UnrealNetwork.h"
#include "GObjectSystem/GNonfixedObject.h"
#include "ServerModule/GameSession/GameSessionMode.h"

#include "Components/SphereComponent.h"


AExocismCircle::AExocismCircle()
{

}


void AExocismCircle::OnOuterZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 1. 서버 권한 체크 (멀티플레이 고려)
    if (!HasAuthority()) return;

    auto* Object = Cast<AGNonfixedObject>(OtherActor);
    if (!Object) return;

    FString ObjectTID = Object->GetNonfixedObjCoreComp()->TID.ToString();
    bool bMatched = false;

    // 2. 현재 남아있는 작은 존들 순회
    for (int32 i = InnerZones.Num() - 1; i >= 0; --i)
    {
        USphereComponent* Zone = InnerZones[i];
        if (Zone && Zone->IsOverlappingActor(OtherActor))
        {
            // 3. TID 매칭 확인
            FName TargetTID = ZoneTIDMap.FindRef(Zone->GetName());
            if (ObjectTID == TargetTID)
            {
                UE_LOG(LogTemp, Log, TEXT("퇴마 구역 매칭 성공: %s"), *Zone->GetName());

                Zone->DestroyComponent();
                InnerZones.RemoveAt(i);

                bMatched = true;
                break;
            }
        }
    }

    if (bMatched)
    {
        CheckExorcismSuccess();
    }
    else
    {
        HandleExorcismFailure();
    }
}


void AExocismCircle::CheckExorcismSuccess()
{
    if (InnerZones.Num() == 0)
    {
        // do clear exocism
    }
}
void AExocismCircle::HandleExorcismFailure()
{
    // GetWorld()-> FinishExorcismFail();
}
