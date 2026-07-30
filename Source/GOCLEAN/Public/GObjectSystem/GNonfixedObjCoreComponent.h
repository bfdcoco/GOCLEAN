// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"


#include "GTypes/GObjectTypes.h"
#include "GTypes/IGInteractable.h"
#include "GObjectSystem/GAdditionalObjFuncComponent.h"

#include "GNonfixedObjCoreComponent.generated.h"


class AGOCLEANCharacter;


DECLARE_MULTICAST_DELEGATE_OneParam(FOnNonfixedObjIneracted, AGOCLEANCharacter*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNonfixedObjStateChanged, ENonfixedObjState, ENonfixedObjState);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GOCLEAN_API UGNonfixedObjCoreComponent : public UActorComponent, public IGInteractable
{
	GENERATED_BODY()


public:	
	// Sets default values for this component's properties
	UGNonfixedObjCoreComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	// Variables //
	// instance ID - ObjectPool에서 재활용 시, 생성된 obj cnt가 instance id로 사용됨
	UPROPERTY(VisibleAnywhere, Replicated, Category = "ID")
	int32 IID;

	// type ID - 현재 오브젝트가 어떤 오브젝트인지 식별하는 ID
	UPROPERTY(EditAnywhere, ReplicatedUsing="OnRep_TID", Category = "ID")
	FName TID;


	// Interaction // 
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_InteractionCnt, Category = "Interaction")
	int32 InteractionCnt;

	FOnNonfixedObjIneracted& GetOnInteractionDelegate() { return OnNonfixedObjInteracted; }

	// interact
	virtual bool CanInteract(FName EquipID, AGOCLEANCharacter* Target) const override;
	virtual void ExecuteInteraction(FName EquipID, AGOCLEANCharacter* Target) override;

	ENonfixedObjState GetState() { return State; }



	// state
	bool ChangeState(ENonfixedObjState ChangedState);

	ENonfixedObjState GetNonfixedObjState() const { return State; }

	FOnNonfixedObjStateChanged& GetOnStateChangedDelegate() { return OnNonfixedObjStateChanged; }



protected:

	// OnReq
	UFUNCTION()
	void OnRep_InteractionCnt();


	// State // 
	// // state - 현재 오브젝트의 상태
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_State, Category = "State")
	ENonfixedObjState State;

	// state
	void OnStateExit(ENonfixedObjState ExitedState);
	void OnStateEnter(ENonfixedObjState EnteredState);
	void OnStateUpdate();


	UFUNCTION()
	void OnRep_State();

	UFUNCTION()
	void OnRep_TID();


private:

	// 인터랙션 발생 시, 해당 오브젝트가 수행해야할 동작 묶음
	FOnNonfixedObjIneracted OnNonfixedObjInteracted;

	FOnNonfixedObjStateChanged OnNonfixedObjStateChanged;

};
