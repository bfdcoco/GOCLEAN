#pragma once

#include "CoreMinimal.h"

// enum 및 포함하는 타입이 언리얼 리플렉션 시스템을 사용하지 않는다면 제거해도 됨
#include "GMapTypes.generated.h"



/*
	Map 용어 정리
	1. Map: 메인 게임이 이루어지는 전체 공간을 의미함. MainLevel로 구현됨.
	2. Zone: Map을 나누는 단위. 하나의 Zone이 하나의 SubLevel로 구현됨.
*/



UENUM(BlueprintType)
enum class EGZoneAttribute : uint8 {
	/*
		enum class EGZoneAttribute
			- 공간 별 속성에 대한 enum class
	*/
	E_Water				UMETA(DisplayName = "Water - 물 속성 공간"),

	E_None				UMETA(DisplayName = "None - 공간 속성 없음"),

	E_MAX				UMETA(Hidden)
};



UENUM(BlueprintType)
enum class EGZoneType : uint8 {
	/*
		enum class EGLevelType
			- 공간 종류에 대한 enum class
			- 방 vs 복도
				- 차이
					- 방: 기획서 상에서 명확하게 명칭이 존재하는 zone
					- 복도: 기획서 상에서 명칭이 존재하지 않는 zone(폐공장 지하 1층) 이나 복도
				- 구분 이유
					- 혹시나 복도에 해당하는 영역은 귀신 patrol이 적용되지 않는 등 효과가 있을까봐
					- 구분이 없다면 room으로 통일할 것
	*/
	E_Room				UMETA(DisplayName = "Room"),
	E_Corridor			UMETA(DisplayName = "Corridor"),

	E_Outdoor			UMETA(DisplayName = "Outdoor"),
	E_Basecamp			UMETA(DisplayName = "Basecamp"),

	E_MAX				UMETA(Hidden)
};



class ATargetPoint;
USTRUCT(BlueprintType)
struct FGZoneData 
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGZoneType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGZoneAttribute Attribute = EGZoneAttribute::E_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActiveCandleEffect = false;


	// player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<AActor>> IncludedPlayerList;


	// ghost
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeGhost = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<ATargetPoint>> GhostPatrolPoints;
};
