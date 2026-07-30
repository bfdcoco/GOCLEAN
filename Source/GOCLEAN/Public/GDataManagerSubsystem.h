// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GDataManagerSubsystem.generated.h"


struct FGObjectDataRow;
struct FGEquipmentDataRow;
struct FGMapDataRow;


/**
*	> Function
* - Load data from datatables
* - Get data table each module
 */
UCLASS()
class GOCLEAN_API UGDataManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()


public:
	static UGDataManagerSubsystem* Get(const UObject* WorldContextObject)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				return GI->GetSubsystem<UGDataManagerSubsystem>();
			}
		}
		return nullptr;
	}


	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	const FGObjectDataRow* GetObjectData(FName ID) const;
	const FGEquipmentDataRow* GetEquipmentData(FName ID) const;
	const FGMapDataRow* GetMapData(FName TID) const;

	const TArray<FGMapDataRow*> GetAllMapDatas() const;


private:
	UPROPERTY()
	TObjectPtr<UDataTable> ObjectDataTable;

	UPROPERTY()
	TObjectPtr<UDataTable> EquipmentDataTable;

	UPROPERTY()
	TObjectPtr<UDataTable> MapDataTable;
	
};
