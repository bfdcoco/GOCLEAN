// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmbientZoneActor.generated.h"

class UBoxComponent;
class USoundBase;


UCLASS()
class GOCLEAN_API AAmbientZoneActor : public AActor
{
	GENERATED_BODY()
	
public:
    AAmbientZoneActor();

    // 존 식별자
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmbientZone")
    FName ZoneId = "Default";

    // 겹칠 때 우선순위
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmbientZone")
    int32 Priority = 0;

    // 랜덤으로 재생할 SFX들(3D)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmbientZone")
    TArray<USoundBase*> RandomSfxList;

    // 랜덤 재생 최소/최대 간격(초)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmbientZone", meta = (ClampMin = "0.1"))
    float MinInterval = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmbientZone", meta = (ClampMin = "0.1"))
    float MaxInterval = 20.0f;

    // 간격마다 재생될 확률(0~1)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmbientZone", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PlayChance = 0.5f;

    // 소리가 플레이어 주변에서 날지
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmbientZone")
    bool bPlayNearPlayer = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmbientZone", meta = (EditCondition = "bPlayNearPlayer", ClampMin = "0.0"))
    float NearPlayerRadius = 250.0f;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AmbientZone")
    TObjectPtr<UBoxComponent> Trigger;

    UFUNCTION()
    void OnTriggerBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    // 매니저 사용 간단 유틸 
    USoundBase* PickRandomSfx() const;

};
