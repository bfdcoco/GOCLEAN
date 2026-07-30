#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GTypes/GObjectTypes.h"

#include "GObjectDataRow.generated.h"


USTRUCT(BlueprintType)
struct FGObjectDataRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    FGObjectDataRow() : Pollution(0.0f), InteractionFinCnt(1), SpawnLayer(0) {}


    // Shared //
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shared")
    FName TID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shared")
    FText DisplayName;


    // Visual //
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TArray<TSoftObjectPtr<UStaticMesh>> SM_MeshAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TArray<TSoftObjectPtr<USkeletalMesh>> SK_MeshAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TArray<TSubclassOf<class AActor>> DEC_MeshAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bUseBothHands;


    // Logic //
    //      default
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
    EGObjectCategory Category;

    //      cleaning
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
    float Pollution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
    TArray<FName> MatchedEquipID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
    int32 InteractionFinCnt;

    //      pickup
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
    FName PickedEquipID;

    //      spawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
    int32 SpawnLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
    FName DerivedObjID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
    int32 DerivedObjCnt;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    TObjectPtr<USoundBase> InteractSoundAsset;

};
