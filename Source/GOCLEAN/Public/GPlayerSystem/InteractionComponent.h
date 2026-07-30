// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GOCLEAN_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()


public:	
	// Sets default values for this component's properties
	UInteractionComponent();

	UPROPERTY(EditAnywhere, Category="Interaction")
	float InteractionRange = 300.0f;

	UObject* GetCurrentTarget() const { return CurrentTarget; }

	bool IsCheckingIncineratorZone() { return bIsCheckingIncineratorZone; }



protected:
	// Called when the game starts
	virtual void BeginPlay() override;



	UPROPERTY(EditAnywhere, Category = "Interaction")
	float IncineratorRange = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	bool bIsCheckingIncineratorZone;



public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	UObject* CurrentTarget;

	void PerformLineTrace();
		
};
