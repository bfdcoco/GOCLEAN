

#pragma once

#include "Engine/DeveloperSettings.h"
#include "GOCLEANSettings.generated.h"


UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "GoClean Project Settings"))
class GOCLEAN_API UGOCLEANSettings : public UDeveloperSettings
{
    GENERATED_BODY()


public:
    // UGOCLEANSettings();


    // DtatTables //
    UPROPERTY(Config, EditAnywhere, Category = "DataTalbes")
    TSoftObjectPtr<UDataTable> ObjectDataTablePath;

    UPROPERTY(Config, EditAnywhere, Category = "DataTalbes")
    TSoftObjectPtr<UDataTable> EquipmentDataTablePath;

    UPROPERTY(Config, EditAnywhere, Category = "DataTalbes")
    TSoftObjectPtr<UDataTable> ThatchedHouseMapDataTablePath;


    // Participant // 
    UPROPERTY(Config, EditAnywhere, Category = "Participant", meta = (ClampMin = "1", ClampMax = "10"))
    int32 ParticipantMax;


    // Equipment //
    UPROPERTY(Config, EditAnywhere, Category="Equipment|Slot", meta=(ClampMin = "1", ClampMax = "10"))
    int32 EquipmentSlotMax;

    UPROPERTY(Config, EditAnywhere, Category="Equipment|Pollution", meta=(ClampMin = "0.0", ClampMax = "100.0"))
    float PollutionMax;

    UPROPERTY(Config, VisibleAnywhere, Category = "Equipment|Pollution|Level", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float PollutionThreshold_LV0;

    UPROPERTY(Config, VisibleAnywhere, Category = "Equipment|Pollution|Level", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float PollutionThreshold_LV1;

    UPROPERTY(Config, VisibleAnywhere, Category = "Equipment|Pollution|Level", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float PollutionThreshold_LV2;

    UPROPERTY(Config, VisibleAnywhere, Category = "Equipment|Pollution|Level", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float PollutionThreshold_LV3;

    // WaterPollution += MopPollution * WaterPollutionByMop
    UPROPERTY(Config, EditAnywhere, Category = "Equipment|Pollution|Water", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float WaterPollutionByMop;

    UPROPERTY(Config, EditAnywhere, Category = "Equipment|Pollution|Water", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float WaterPollutionByAutoMop;


    // Getter // 
    static const UGOCLEANSettings* Get() { return GetDefault<UGOCLEANSettings>(); }


    // Editor //
    void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);

};
