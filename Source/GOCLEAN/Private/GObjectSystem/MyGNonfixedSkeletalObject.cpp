// Fill out your copyright notice in the Description page of Project Settings.


#include "GObjectSystem/MyGNonfixedSkeletalObject.h"

#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"

#include "GTypes/DataTableRow/GObjectDataRow.h"
#include "GDataManagerSubsystem.h"
#include "GObjectSystem/GNonfixedObjCoreComponent.h"

#include <Kismet/KismetSystemLibrary.h>


AGNonfixedSkeletalObject::AGNonfixedSkeletalObject()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;


	bReplicates = true;
	SetReplicateMovement(true);


	RootPrimitive = CreateDefaultSubobject<USkeletalMeshComponent>("RootPrimitive");
	SetRootComponent(RootPrimitive);

	RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);


	InteractionVolume = CreateDefaultSubobject<UBoxComponent>("InteractionVolume");
	InteractionVolume->SetupAttachment(RootPrimitive);

	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Overlap);
	InteractionVolume->SetHiddenInGame(false);

	InteractionVolume->SetCollisionResponseToChannel(ECC_GInteractable, ECR_Block);
	InteractionVolume->SetSimulatePhysics(false);


	CoreComp = CreateDefaultSubobject<UGNonfixedObjCoreComponent>("CoreComponent");

}

void AGNonfixedSkeletalObject::UpdateInteractionBounds()
{
	if (!RootPrimitive || !InteractionVolume) return;

	USkeletalMesh* MeshAsset = RootPrimitive->GetSkeletalMeshAsset();
	if (!MeshAsset) return;

	const FBox LocalBox = MeshAsset->GetImportedBounds().GetBox();

	const FVector Center = LocalBox.GetCenter();
	const FVector Extent = LocalBox.GetExtent();

	InteractionVolume->SetBoxExtent(Extent);

	InteractionVolume->SetRelativeLocation(Center);
}


void AGNonfixedSkeletalObject::BeginPlay()
{
	Super::BeginPlay();

	RootPrimitive->SetSimulatePhysics(false);

	RootPrimitive->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	RootPrimitive->SetAnimInstanceClass(nullptr);
	RootPrimitive->bPauseAnims = true;

	RootPrimitive->SetAllBodiesBelowSimulatePhysics(RootPrimitive->GetBoneName(0), true, true);
	RootPrimitive->SetPhysicsBlendWeight(1.0f);
	RootPrimitive->SetEnableGravity(true);
	RootPrimitive->WakeAllRigidBodies();

	UpdateInteractionBounds();
}


void AGNonfixedSkeletalObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//FString str = (RootPrimitive->IsSimulatingPhysics()) ? TEXT("Sim ON") : TEXT("Sim OFF");
	//UKismetSystemLibrary::PrintString(GetWorld(), str, true, true, FLinearColor::Green, 2.0f);

	//if (HasAuthority())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Server Tick"));
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Client Tick"));
	//}
}

