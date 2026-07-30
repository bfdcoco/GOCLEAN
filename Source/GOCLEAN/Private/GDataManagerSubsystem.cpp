// Fill out your copyright notice in the Description page of Project Settings.


#include "GDataManagerSubsystem.h"

#include "GTypes/DataTableRow/GObjectDataRow.h"
#include "GTypes/DataTableRow/GEquipmentDataRow.h"
#include "GTypes/DataTableRow/GMapDataRow.h"
#include "GOCLEANSettings.h"
#include "../GOCLEAN.h"


const FGObjectDataRow* UGDataManagerSubsystem::GetObjectData(FName ID) const
{
    if (!ObjectDataTable)
    {
        // GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange, FString::Printf(TEXT("Data not found for TID: %s"), *ID.ToString()));
        return nullptr;
    }

    static const FString ContextString(TEXT("Object Data Lookup"));
    return ObjectDataTable->FindRow<FGObjectDataRow>(ID, ContextString);
}

const FGEquipmentDataRow* UGDataManagerSubsystem::GetEquipmentData(FName ID) const
{
    if (!EquipmentDataTable) return nullptr;

    static const FString ContextString(TEXT("Object Data Lookup"));
    return EquipmentDataTable->FindRow<FGEquipmentDataRow>(ID, ContextString);
}

const FGMapDataRow* UGDataManagerSubsystem::GetMapData(FName ID) const
{
    if (!MapDataTable) return nullptr;

    static const FString ContextString(TEXT("Object Data Lookup"));
    return MapDataTable->FindRow<FGMapDataRow>(ID, ContextString);
}

const TArray<FGMapDataRow*> UGDataManagerSubsystem::GetAllMapDatas() const
{
    TArray<FGMapDataRow*> Datas;

    if (!MapDataTable)
    {
        UE_LOG(LogMap, Warning, TEXT("MapDataTable does not registered!"));
        return Datas;
    }

    FString ContextString(TEXT("GetAllMapDatasContext"));
    MapDataTable->GetAllRows<FGMapDataRow>(ContextString, Datas);

    return Datas;
}


void UGDataManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const UGOCLEANSettings* Settings = GetDefault<UGOCLEANSettings>();
    if (Settings)
    {
        ObjectDataTable = Settings->ObjectDataTablePath.LoadSynchronous();
        EquipmentDataTable = Settings->EquipmentDataTablePath.LoadSynchronous();

        // 선택한 맵에 따라 로드할 테이블을 변경
        MapDataTable = Settings->ThatchedHouseMapDataTablePath.LoadSynchronous();
    }
}
