#pragma once

#define ECC_GInteractable ECC_GameTraceChannel2
#define EEC_GIncinerator ECC_GameTraceChannel3
#define EEC_GIncinerable ECC_GameTraceChannel4

#include "CoreMinimal.h"

#include "IGInteractable.generated.h"


class AGOCLEANCharacter;


// 언리얼 엔진이 해당 인터페이스가 존재함을 인식시키기 위해 정의
// UObject 시스템에 현재 인터페이스를 등록해주는 역할 / 메타데이터
UINTERFACE(MinimalAPI)
class UGInteractable : public UInterface { GENERATED_BODY() };

// 실제 인터페이스 구현체
class IGInteractable {
    GENERATED_BODY()


public:
    virtual bool CanInteract(FName EquipID, AGOCLEANCharacter* Target) const = 0;
    virtual void ExecuteInteraction(FName EquipID, AGOCLEANCharacter* Target) = 0;
};
