// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTypes/IGInteractable.h"

#include "GDoorway.generated.h"



UCLASS()
class GOCLEAN_API AGDoorway : public AActor
{
	GENERATED_BODY()
	


	// Default & Replication
public:	
	AGDoorway();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void CloseDoor() 
	{
		bIsClosed = true;
		OnRep_IsClosed();
	}

	void OpenDoor()
	{
		bIsClosed = false;
		OnRep_IsClosed();
	}

	bool IsClosed() { return bIsClosed; }
	bool CanClose() { return bCanClose; }


protected:
	virtual void BeginPlay() override;



	// Variables
protected:
	// only host
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UBoxComponent* TriggerBox;

	// replicated
	UPROPERTY(EditAnywhere, Replicated, Category = "Zone")
	FName FromZone;

	UPROPERTY(EditAnywhere, Replicated, Category = "Zone")
	FName ToZone;

	UPROPERTY(EditAnywhere, Replicated, Category = "Status")
	bool bCanClose;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_IsClosed, Category = "Status")
	bool bIsClosed;

	// door bp를 통해 문 회전을 보여줘야 함
	// 1안. door bp에 회전을 만들고, 해당 bp를 property로 등록시킨다
	//		- open 이벤트 발생 시, bp에 등록된 회전 기능을 실행한다.
	// 2안. skeletal mesh를 넣어준다. 
	//		- 이렇게 되면 door 재배치해야할 수도



	// Funcitons
private:
	// only server
	void EnterPlayer(AActor* PlayerActor);
	void ExitPlayer(AActor* PlayerActor);

	void EnterGhost(AActor* GhostActor);
	void ExitGhost(AActor* GhostActor);

	// internal functions
	int32 FindPlayerSessionIndex(AActor* PlayerActor);

	// TEST
	void TEST_OpenAllDoors();


protected:
	// Trigger
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// OnRep
	UFUNCTION()
	void OnRep_IsClosed();

	UPROPERTY(EditAnywhere, Category = "Door")
	class AGDoor* LinkedDoor;

};



UCLASS()
class GOCLEAN_API AGDoor : public AActor, public IGInteractable
{
	GENERATED_BODY()


	// Default & Replication
public:
	AGDoor() {};

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void ReceiveOnDoorStateChanged(bool bClosed);

	virtual bool CanInteract(FName EquipID, AGOCLEANCharacter* Target) const;
	virtual void ExecuteInteraction(FName EquipID, AGOCLEANCharacter* Target);


	UPROPERTY(EditAnywhere, Category = "Door")
	class AGDoorway* OwnerDoorway;


	UPROPERTY(Replicated="ReceiveOnDoorStateChanged")
	bool bIsClosed = true;

};
