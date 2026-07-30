/**
 * @class AGhostBase
 * @brief Ghost stats component를 기반으로 하여, 이벤트 수행 조건 판단 및, 수행의 주체로서 존재
 *
 * **[Core functions]**
 * - CheckBehaviorEventCondition(): 플레이어 정신 오염도 기반, 행동 이벤트 수행 조건 판단
 * - PerformBehaviorEvent(): 공통 & 단서 행동 이벤트 수행
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GEnemy/StatsComponent/GhostStatsComponent.h"
#include "GEnemy/EvidenceBehaviors/Base/UEvidenceBehavior.h"
#include "GEnemy/CommonBehaviors/Base/CommonBehavior.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AudioComponent.h"
#include "GhostBase.generated.h"

//JSH FLAH AGHOSTAIController -> Revision: AGhostAIController
class AGhostAIController;
class UCommonBehavior;
class UEvidenceBehavior;

UCLASS(Abstract)
class GOCLEAN_API AGhostBase : public ACharacter
{
	GENERATED_BODY()


public:
	AGhostBase();



	// Using in GhostAIController.h/.cpp Patrol
	UPROPERTY(EditAnywhere, Category = "Patrol Points");
	TArray<TObjectPtr<AActor>> PatrolPoints;

	int32 CurrentPatrolIndex;

	
	// Sound //
	UPROPERTY()
	TObjectPtr<UAudioComponent> RageLoopAudio;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> RageCue;
	UPROPERTY()
	TObjectPtr<UAudioComponent> ChaseLoopAudio;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> ChaseCue;
	UPROPERTY()
	TObjectPtr<UAudioComponent> OnHuntedLoopAudio;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> OnHuntedCue;

	void PlayRageSound();
	void StopRageSound();
	void PlayChaseSound();
	void StopChaseSound();

	// Rage
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayRageSound();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopRageSound();

	// Chase
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayChaseSound();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopChaseSound();

	// OnHunted
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayOnHuntedSound();
	
	// Server //
	UFUNCTION(Server, Reliable)
	void Server_RequestSetVisible(bool IsVisible);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetVisible(bool IsVisible);

protected:
	// Behaviors array //
	UPROPERTY(VisibleAnywhere, Category = "Behaviors");
	TArray<TObjectPtr<UCommonBehavior>> CommonBehaviors;

	UPROPERTY(VisibleAnywhere, Category = "Behaviors");
	TArray<TObjectPtr<UEvidenceBehavior>> EvidenceBehaviors;

	// Stats Component //
	UPROPERTY(VisibleAnywhere, Category = "Stats")
	TObjectPtr<UGhostStatsComponent> StatsComp;



	// Overrided //
	void BeginPlay() override;
	void Tick(float DeltaTime) override;

	// Behavior event //
	void CheckBehaviorEventCondition();
	void PerformBehaviorEvent();


	// Player sanity //
	float SanityCorruptionRate;

	// Behavior event //
	float BehaviorEventCycleDelay;
	bool bCanSetBehaviourEventCycleTimer;
	FTimerHandle GhostBehaviorCycleHandle;

private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AGhostAIController> GhostAIController;
};
