// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GObjectSystem/GFixedObject.h"
#include "ExocismCircle.generated.h"


UCLASS()
class GOCLEAN_API AExocismCircle : public AGFixedObject
{
	GENERATED_BODY()
	

public:
	AExocismCircle();


    // 대형 외곽 존
    UPROPERTY(VisibleAnywhere, Category = "Exorcism")
    class USphereComponent* OuterZone;

    // 작은 존들을 관리할 배열 (블루프린트에서 편집 가능하게 설정)
    UPROPERTY(VisibleAnywhere, Category = "Exorcism")
    TArray<class USphereComponent*> InnerZones;

    // 각 존에 매핑되는 TID 정보 (Key: 컴포넌트 이름 또는 포인터, Value: TID)
    UPROPERTY(EditAnywhere, Category = "Exorcism")
    TMap<FString, FName> ZoneTIDMap{
        { "TPile1", "Obj_TPile" },
        {"TPile2", "Obj_TPile"},
        {"TPile3", "Obj_TPile"},
        {"Fetish", "Obj_FPot"},
    };

    // 외곽 존 오버랩 이벤트 함수
    UFUNCTION()
    void OnOuterZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    void CheckExorcismSuccess();
    void HandleExorcismFailure();

};
