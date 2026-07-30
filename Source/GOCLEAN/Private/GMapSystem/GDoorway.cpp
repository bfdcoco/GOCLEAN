// Fill out your copyright notice in the Description page of Project Settings.


#include "GMapSystem/GDoorway.h"

#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"

#include "GMapSystem/Server/GMapManager.h"
#include "GCharacter/GOCLEANCharacter.h"
#include "GEnemy/Base/GhostBase.h"
#include "GOCLEANSettings.h"
#include "../../GOCLEAN.h"



// Constructor // 
AGDoorway::AGDoorway()
{
	bReplicates = true;
	// AActor::SetReplicateMovement(true);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}



// Replication // 
void AGDoorway::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGDoorway, FromZone);
	DOREPLIFETIME(AGDoorway, ToZone);
	DOREPLIFETIME(AGDoorway, bCanClose);
	DOREPLIFETIME(AGDoorway, bIsClosed);
}

void AGDoorway::OnRep_IsClosed()
{
	UE_LOG(LogTemp, Warning, TEXT("Client: OnRep! Door state: %d"), bIsClosed);

	// do rotation anim
	if (LinkedDoor)
	{
		LinkedDoor->ReceiveOnDoorStateChanged(bIsClosed);
	}
}



// Life cycle & Engine event //
void AGDoorway::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AGDoorway::OnOverlapEnd);

		// TEST_OpenAllDoors();
	}
	else
	{
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TriggerBox->SetGenerateOverlapEvents(false);

		// TriggerBox->DestroyComponent();
	}
}
void AGDoorway::TEST_OpenAllDoors()
{
	FTimerHandle TestTimerHandle;

	// 3초마다 람다 함수를 실행해서 강제로 문 상태를 뒤집어버립니다!
	GetWorld()->GetTimerManager().SetTimer(TestTimerHandle, FTimerDelegate::CreateLambda([this]()
	{
		OpenDoor(); // 변수 변경! (이 순간 클라이언트로 날아감)
		UE_LOG(LogTemp, Warning, TEXT("[Server] Door close force: %d"), bIsClosed);

	}), 3.0f, false); // 3초마다 반복 (true)
}

void AGDoorway::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 1. check closed
	if (bIsClosed) return;


	// 2. check other actor's effectiveness
	if (!OtherActor) return;

	ACharacter* TargetChar = Cast<ACharacter>(OtherActor);
	if (!TargetChar) return;


	// 3. check character type
	bool bIsPlayerChar;
	if (TargetChar->IsA(AGOCLEANCharacter::StaticClass()))
	{
		bIsPlayerChar = true;
	}
	else if (TargetChar->IsA(AGhostBase::StaticClass()))
	{
		bIsPlayerChar = false;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GDoorway::TriggerEvent] Unkown type's charater detected."));
		return;
	}


	// 4. set vector
	FVector DoorForward = GetActorRightVector();
	FVector CharVelocity = TargetChar->GetVelocity();
	
	FVector MoveDir = CharVelocity.GetSafeNormal();


	// 5. caculate dot product
	float DotResult = FVector::DotProduct(DoorForward, MoveDir);


	// 6. do process
	if (bIsPlayerChar)
	{
		if (DotResult > 0.0f)
		{
			UE_LOG(LogMap, Log, TEXT("[GDoorway::TriggerEvent] Player Entered to %s"), *ToZone.ToString());
			UE_LOG(LogMap, Log, TEXT("[GDoorway::TriggerEvent] Player Exit to %s"), *FromZone.ToString());
			EnterPlayer(OtherActor);
		}
		else
		{
			UE_LOG(LogMap, Log, TEXT("[GDoorway::TriggerEvent] Player Entered to %s"), *FromZone.ToString());
			UE_LOG(LogMap, Log, TEXT("[GDoorway::TriggerEvent] Player Exit to %s"), *ToZone.ToString());
			ExitPlayer(OtherActor);
		}
	}
	else
	{
		if (DotResult < 0.0f)
		{
			UE_LOG(LogMap, Log, TEXT("[GDoorway::TriggerEvent] Ghost Entered to %s"), *ToZone.ToString());
			UE_LOG(LogMap, Log, TEXT("[GDoorway::TriggerEvent] Ghost Exit to %s"), *FromZone.ToString());
			EnterGhost(OtherActor);
		}
		else
		{
			UE_LOG(LogMap, Log, TEXT("[GDoorway::TriggerEvent] Ghost Entered to %s"), *FromZone.ToString());
			UE_LOG(LogMap, Log, TEXT("[GDoorway::TriggerEvent] Ghost Exit to %s"), *ToZone.ToString());
			ExitGhost(OtherActor);
		}
	}
}



// Game event // 
void AGDoorway::EnterPlayer(AActor* PlayerActor)
{
	// 1. find player's session index and check effectiveness
	//int32 PlayerIndex = FindPlayerSessionIndex(PlayerActor);
	//if (PlayerIndex < 0 && PlayerIndex >= UGOCLEANSettings::Get()->ParticipantMax)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("[GDoorway::TriggerEvent] Cannot found matched player in session list"));
	//	return;
	//}

	UWorld* World = GetWorld();
	if (!World) return;

	auto* MapManager = World->GetSubsystem<UGMapManager>();
	if (!MapManager) return;


	// 2. remove player index at FromZone's included player list
	MapManager->PlayerExitedZone(FromZone, PlayerActor);


	// 3. add player index at ToZone's included player list
	MapManager->PlayerEnteredZone(ToZone, PlayerActor);
}

void AGDoorway::ExitPlayer(AActor* PlayerActor)
{
	// 1. find player's session index and check effectiveness
	//int32 PlayerIndex = FindPlayerSessionIndex(PlayerActor);
	//if (PlayerIndex < 0 && PlayerIndex >= UGOCLEANSettings::Get()->ParticipantMax)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("[GDoorway::TriggerEvent] Cannot found matched player in session list"));
	//	return;
	//}

	UWorld* World = GetWorld();
	if (!World) return;

	auto* MapManager = World->GetSubsystem<UGMapManager>();
	if (!MapManager) return;


	// 2. remove player index at ToZone's included player list
	MapManager->PlayerExitedZone(ToZone, PlayerActor);


	// 3. add player index at FromZone's included player list
	MapManager->PlayerEnteredZone(FromZone, PlayerActor);
}

void AGDoorway::EnterGhost(AActor* GhostActor)
{
	// if parameter does not use, remove parameter

	UWorld* World = GetWorld();
	if (!World) return;

	auto* MapManager = World->GetSubsystem<UGMapManager>();
	if (!MapManager) return;

	// 1. set FromZone's bIncludeGhost to FALSE
	MapManager->GhostExitedZone(FromZone, GhostActor);

	// 2. set ToZone's bIncludeGhost to TRUE
	MapManager->GhostEnteredZone(ToZone, GhostActor);
}
void AGDoorway::ExitGhost(AActor* GhostActor)
{
	// if parameter does not use, remove parameter

	UWorld* World = GetWorld();
	if (!World) return;

	auto* MapManager = World->GetSubsystem<UGMapManager>();
	if (!MapManager) return;

	// 1. set ToZone's bIncludeGhost to FALSE
	MapManager->GhostExitedZone(ToZone, GhostActor);

	// 2. set FromZone's bIncludeGhost to TRUE	
	MapManager->GhostEnteredZone(FromZone, GhostActor);
}

int32 AGDoorway::FindPlayerSessionIndex(AActor* PlayerActor)
{
	int32 Index = -1;


	// 1. find same player character in server's session manager


	// 2. get the player's index


	return Index;
}




bool AGDoor::CanInteract(FName EquipID, AGOCLEANCharacter* Target) const
{
	if (!OwnerDoorway) return true;

	return OwnerDoorway->CanClose();
}

void AGDoor::ExecuteInteraction(FName EquipID, AGOCLEANCharacter* Target)
{
	if (!OwnerDoorway)
	{
		bIsClosed = !bIsClosed;
		ReceiveOnDoorStateChanged(bIsClosed);
		return;
	}

	OwnerDoorway->IsClosed() ? OwnerDoorway->OpenDoor() : OwnerDoorway->CloseDoor();
}

void AGDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGDoor, bIsClosed);
}

