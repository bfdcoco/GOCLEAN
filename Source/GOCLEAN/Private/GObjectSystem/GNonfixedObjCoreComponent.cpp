// Fill out your copyright notice in the Description page of Project Settings.


#include "GObjectSystem/GNonfixedObjCoreComponent.h"
#include "GObjectSystem/GNonfixedObject.h"
#include "../../GOCLEAN.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UGNonfixedObjCoreComponent::UGNonfixedObjCoreComponent()
{
	TID = "";
	SetIsReplicatedByDefault(true);

	PrimaryComponentTick.bCanEverTick = false;
}

void UGNonfixedObjCoreComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGNonfixedObjCoreComponent, IID);
	DOREPLIFETIME(UGNonfixedObjCoreComponent, TID);
	DOREPLIFETIME(UGNonfixedObjCoreComponent, InteractionCnt);
	DOREPLIFETIME(UGNonfixedObjCoreComponent, State);
}


// Called when the game starts
void UGNonfixedObjCoreComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UGNonfixedObjCoreComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


// interface
bool UGNonfixedObjCoreComponent::CanInteract(FName EquipID, AGOCLEANCharacter* Target) const
{
	// check equip id

	return true;
}

void UGNonfixedObjCoreComponent::ExecuteInteraction(FName EquipID, AGOCLEANCharacter* Target)
{
	//if (!GetOwner()->HasAuthority())
	//	return;

	InteractionCnt++;
	// InteractionCnt에 따른 동작은 아래 델리게이트에서
	OnNonfixedObjInteracted.Broadcast(Target);

	UE_LOG(LogTemp, Log, TEXT("[GEquipment] %s Interacted! Count: %d, Tool: %s"), *GetName(), InteractionCnt, *EquipID.ToString());
}


void UGNonfixedObjCoreComponent::OnRep_InteractionCnt()
{
	// update visual
}

void UGNonfixedObjCoreComponent::OnRep_State()
{
	// update visual
	UE_LOG(LogGObject, Log, TEXT("Object State was change!: %s"), *UEnum::GetValueAsString(State));

	AGNonfixedObject* Owner = Cast<AGNonfixedObject>(GetOwner());
	if (Owner)
	{
		Owner->UpdatePhysicsByState();
		Owner->UpdateVisualByState();
	}
}

void UGNonfixedObjCoreComponent::OnRep_TID()
{
	// update visual
	UE_LOG(LogGObject, Log, TEXT("Object State was change!: %s"), *TID.ToString());

	AGNonfixedObject* Owner = Cast<AGNonfixedObject>(GetOwner());
	if (Owner)
	{
		FGNonfixedObjData TempData{
			IID,
			TID,
			Owner->GetActorLocation(),
			Owner->GetActorRotation(),
			ENonfixedObjState::E_Static,
			true
		};

		Owner->SetObjectData(TempData);
	}
}


// state
bool UGNonfixedObjCoreComponent::ChangeState(ENonfixedObjState ChangedState)
{
	ENonfixedObjState PrevState = State;
	if (PrevState == ChangedState) return false;

	OnStateExit(PrevState);
	State = ChangedState;
	OnStateEnter(ChangedState);

	OnNonfixedObjStateChanged.Broadcast(PrevState, ChangedState);
	OnRep_State();

	return true;
}

void UGNonfixedObjCoreComponent::OnStateExit(ENonfixedObjState ExitedState)
{
	// 추후 OnfixedObjStateChanged 델리게이트를 Exit과 Enter로 분할
}

void UGNonfixedObjCoreComponent::OnStateEnter(ENonfixedObjState EnteredState)
{
	// 추후 OnfixedObjStateChanged 델리게이트를 Exit과 Enter로 분할
}
