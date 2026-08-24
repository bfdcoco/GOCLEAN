#include "GEnemy/GhostAIController.h"
#include "GEnemy/Base/GhostBase.h"
#include "GCharacter/GOCLEANCharacter.h"


// Getter //
//float AGhostAIController::GetPlayerSanityCorruptionRate() const { return PlayersSanityCorruptionRate; };

// Server //
void AGhostAIController::UpdateAlivePlayerList()
{
	if (HasAuthority() == false) return;
	
	AlivePlayers.Empty();

	AGameSessionState* SessionState = Cast<AGameSessionState>(GetWorld()->GetGameState());
	if (SessionState == nullptr) return;

	UE_LOG(LogTemp, Warning, TEXT("AlivePlayers Num: %d"), AlivePlayers.Num());
	for (APlayerState* PS : SessionState->PlayerArray)
	{
		int32 SeatIndex = SessionState->GetSeatIndexOfPlayerState(PS);

		APawn* PlayerPawn = SessionState->GetPawnBySeat(SeatIndex);
		if (PlayerPawn == nullptr) continue;

		AGOCLEANCharacter* PlayerCharacter = Cast<AGOCLEANCharacter>(PlayerPawn);
		if (PlayerCharacter == nullptr) continue;

		if (PlayerCharacter->GetPlayerCurrentLife() <= 0) continue;

		AlivePlayers.Add(PlayerCharacter);
	}
}

void AGhostAIController::FindTarget()
{
	if (AlivePlayers.Num() == 0) return;

	// JSH Tmp: Random Search
	int32 RandomIndex = FMath::RandRange(0, AlivePlayers.Num() - 1);
	TargetPlayer = AlivePlayers[RandomIndex];
}

float AGhostAIController::CalculateAverageSanityCorruptionRate()
{
	if (AlivePlayers.Num() == 0) return 0.f;

	float TotalSanityCorruption = 0.f;

	for (AGOCLEANCharacter* PlayerCharacter : AlivePlayers)
	{
		if (PlayerCharacter == nullptr) continue;

		float SanityCorruption = 100.0f - PlayerCharacter->GetPlayerCurrentSanity();
		TotalSanityCorruption += SanityCorruption;
	}

	return (TotalSanityCorruption / AlivePlayers.Num());
}

void AGhostAIController::CalculateAveragePlayerSanity()
{
	if (AlivePlayers.Num() == 0) return;

	int32 TotalPlayerSanity = 0;

	for(AGOCLEANCharacter* PlayerCharacter : AlivePlayers)
	{
		if (PlayerCharacter == nullptr) continue;

		TotalPlayerSanity += PlayerCharacter->GetPlayerCurrentSanity();
	}

	AveragePlayerSanity = static_cast<int>(TotalPlayerSanity / AlivePlayers.Num());
}

bool AGhostAIController::CheckBehaviorEventCondition()
{
	CalculateAveragePlayerSanity();
	int32 EventValue = FMath::RandRange(0, MaxEventValueRange);
	int32 BehaviorCheckValue = AveragePlayerSanity + ActivityLevel + ActivityLevelModifier;

	if (BehaviorCheckValue <= 70 && EventValue == 15) return true;
	else return false;
}

void AGhostAIController::InitActivityLevel()
{
	ActivityLevel = FMath::RandRange(0, 5);
}

// Overrided //
void AGhostAIController::BeginPlay()
{
	Super::BeginPlay();

	MoveToPatrolPoint();
}

void AGhostAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if(!bIsRageEvent) 
		CheckRageEventCondition();*/
}

void AGhostAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	//GetWorld()->GetTimerManager().SetTimer(CheckPlayerSanityCorruptionHandle, this, &AGhostAIController::CheckPlayerSanityCorruptionRate, 5.0f, true);

	bIsRageEvent = false;
	bIsChasing = false;
	bIsPatrolling = true;

	ManifestRadius = 500.0f;
	HuntRadius = 150.0f;
	
	UpdateAlivePlayerList();

	MaxEventValueRange = 100;
	InitActivityLevel();
	ActivityLevelModifier = 0;
	bCanRageEvent = true;

	GetWorld()->GetTimerManager().SetTimer(CheckRageEventConditionHandle, this, &AGhostAIController::CheckRageEventCondition, 10.0f, true);
}


// Check player sanity //
//void AGhostAIController::CheckPlayerSanityCorruptionRate()
//{
//	PlayersSanityCorruptionRate = CalculateAverageSanityCorruptionRate();
//	UpdatePlayerList();
//	// JSH Flag: Sanity
//	UE_LOG(LogTemp, Warning, TEXT("Player's current sanity corruption rate: %f"), (PlayersSanityCorruptionRate));
//
//	//PlayerSanityCorruptionRate = (100.0f - Player->GetPlayerCurrentSanity());
//}


// State //
void AGhostAIController::MoveToPatrolPoint()
{
	UE_LOG(LogTemp, Warning, TEXT("Move to next patrol point"));
	AGhostBase* GhostCharacter = Cast<AGhostBase>(GetPawn());
	if (GhostCharacter == nullptr || GhostCharacter->PatrolPoints.Num() == 0) return;

	AActor* TargetPoint = GhostCharacter->PatrolPoints[GhostCharacter->CurrentPatrolIndex];
	if (TargetPoint == nullptr) return;

	if (bIsPatrolling == true && bIsChasing == false) {
		MoveToActor(TargetPoint);
	}

	GetWorld()->GetTimerManager().SetTimer(CheckArrivalToCurrentPointHandle, this, &AGhostAIController::CheckArrivalCurrentPatrolPoint, 1.0f, true);
}

void AGhostAIController::CheckArrivalCurrentPatrolPoint()
{
	AGhostBase* GhostCharacter = Cast<AGhostBase>(GetPawn());
	if (GhostCharacter == nullptr || GhostCharacter->PatrolPoints.Num() == 0) return;

	AActor* TargetPoint = GhostCharacter->PatrolPoints[GhostCharacter->CurrentPatrolIndex];
	if (TargetPoint == nullptr) return;

	float Distance = FVector::Dist(GhostCharacter->GetActorLocation(), TargetPoint->GetActorLocation());
	if (Distance < 50.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(CheckArrivalToCurrentPointHandle);

		GhostCharacter->CurrentPatrolIndex = (GhostCharacter->CurrentPatrolIndex + 1) % (GhostCharacter->PatrolPoints.Num());
		MoveToPatrolPoint();
	}
}

void AGhostAIController::CheckRageEventCondition()
{
	if (bIsRageEvent || bCanRageEvent == false || bIsUnendingRageEvent) return;

	CalculateAveragePlayerSanity();
	if (AveragePlayerSanity >= 80) return;

	if ((FMath::RandRange(0, 1) == 1))
	{
		AGhostBase* GhostCharacter = Cast<AGhostBase>(GetPawn());
		if (GhostCharacter == nullptr) return;

		const int32 Insanity = 100 - AveragePlayerSanity;
		const int32 RageCheckValue = Insanity + GhostCharacter->GetRageModifier();

		if (RageCheckValue >= 75)
		{
			if (FMath::RandRange(0, 2) == 1)
			{
				Cast<AGhostBase>(GetPawn())->Multicast_PlayRageSound();
				bIsRageEvent = true;
				bCanRageEvent = false;
				StartChase();
			}
		}
		else if ((40 <= RageCheckValue) && (RageCheckValue < 75))
		{
			if (FMath::RandRange(0, 4) == 1)
			{
				Cast<AGhostBase>(GetPawn())->Multicast_PlayRageSound();
				bIsRageEvent = true;
				bCanRageEvent = false;
				StartChase();
			}
		}
		else
		{
			return;
		}
	}

	return;
}

void AGhostAIController::StartUnendingRageEvent()
{
	bIsUnendingRageEvent = true;
	StartChase();
}

void AGhostAIController::StartChase()
{
	UpdateAlivePlayerList();
	bIsChasing = true;
	bIsPatrolling = false;

	FindTarget();
	if (TargetPlayer == nullptr) return;

	ChasePlayer(TargetPlayer);
}

void AGhostAIController::ChasePlayer(AActor* TargetPlayerCharacter)
{
	if (TargetPlayerCharacter == nullptr) return;

	MoveToActor(TargetPlayerCharacter);

	if(bIsRageEvent) GetWorld()->GetTimerManager().SetTimer(ChasingPlayerHandle, this, &AGhostAIController::PlayerHunt, 0.2f, true);
	else if(bIsUnendingRageEvent) GetWorld()->GetTimerManager().SetTimer(ChasingPlayerHandle, this, &AGhostAIController::EndlessPlayerHunt, 0.2f, true);
}

void AGhostAIController::PlayerHunt()
{
	if (!bIsRageEvent) return;

	//UE_LOG(LogTemp, Warning, TEXT("Hunting Player"));
	AGhostBase* GhostCharacter = Cast<AGhostBase>(GetPawn());
	if (GhostCharacter == nullptr) return;

	// JSH TODO: With Server
	if (TargetPlayer == nullptr) return;

	float Distance = FVector::Dist(GhostCharacter->GetActorLocation(), TargetPlayer->GetActorLocation());
	if (Distance < ManifestRadius)
	{
		Cast<AGhostBase>(GetPawn())->Multicast_PlayChaseSound();
		GhostCharacter->Server_RequestSetVisible(true);
	}

	if (Distance < HuntRadius)
	{
		Cast<AGhostBase>(GetPawn())->Multicast_StopChaseSound();
		Cast<AGhostBase>(GetPawn())->Multicast_StopRageSound();
		Cast<AGhostBase>(GetPawn())->Multicast_PlayOnHuntedSound();
		GhostCharacter->Server_RequestSetVisible(false);
		
		Cast<AGOCLEANCharacter>(TargetPlayer)->Server_RequestOnHunted();

		UE_LOG(LogTemp, Warning, TEXT("Player Hunted"));

		GetWorld()->GetTimerManager().ClearTimer(ChasingPlayerHandle);
		bIsRageEvent = false;
		bIsChasing = false;
		bIsPatrolling = true;
		//PlayersSanityCorruptionRate = 0;

		GetWorld()->GetTimerManager().SetTimer(EnableRageEventTriggerHandle, this, &AGhostAIController::EnableRageTrigger, GhostCharacter->GetRageCooldown(), false);

		MoveToPatrolPoint();
	}
}
void AGhostAIController::Server_RequestPlayerHunt_Implementation()
{
	Multicast_PlayerHunt();
}
void AGhostAIController::Multicast_PlayerHunt_Implementation()
{
	PlayerHunt();
}
void AGhostAIController::Server_RequestSetVisible_Implementation(bool IsVisible)
{
	Multicast_SetVisible(IsVisible);
}
void AGhostAIController::Multicast_SetVisible_Implementation(bool IsVisible)
{
	Cast<ACharacter>(GetPawn())->GetMesh()->SetHiddenInGame(!IsVisible);
}

void AGhostAIController::EndlessPlayerHunt()
{
	if (!bIsUnendingRageEvent) return;

	//UE_LOG(LogTemp, Warning, TEXT("Hunting Player"));
	AGhostBase* GhostCharacter = Cast<AGhostBase>(GetPawn());
	if (GhostCharacter == nullptr) return;

	// JSH TODO: With Server
	AActor* TargetPlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (TargetPlayerCharacter == nullptr) return;

	float Distance = FVector::Dist(GhostCharacter->GetActorLocation(), TargetPlayerCharacter->GetActorLocation());
	if (Distance < ManifestRadius)
	{
		GhostCharacter->Multicast_SetVisible(true);
	}

	if (Distance < HuntRadius)
	{
		GhostCharacter->Multicast_SetVisible(false);

		Cast<AGOCLEANCharacter>(TargetPlayerCharacter)->Server_RequestOnHunted();

		UE_LOG(LogTemp, Warning, TEXT("Player Hunted"));

		GetWorld()->GetTimerManager().ClearTimer(ChasingPlayerHandle);
		bIsChasing = false;
		bIsPatrolling = true;

		// JSH Temp: Player sanity reset
		//AGOCLEANCharacter* Player = Cast<AGOCLEANCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

		if (bIsUnendingRageEvent) EndlessPlayerHunt();
	}
}

void AGhostAIController::OnRep_IsChasing()
{
	AGhostBase* Ghost = Cast<AGhostBase>(GetPawn());
	if (!Ghost) return;

	if (bIsChasing)
		Ghost->Multicast_PlayChaseSound();
	else
		Ghost->Multicast_StopChaseSound();
}

void AGhostAIController::OnRep_IsRageEvent()
{
	AGhostBase* Ghost = Cast<AGhostBase>(GetPawn());
	if (!Ghost) return;

	if (bIsRageEvent)
		Ghost->Multicast_PlayRageSound();
	else
		Ghost->Multicast_StopRageSound();
}

void AGhostAIController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGhostAIController, bIsPatrolling);
	DOREPLIFETIME(AGhostAIController, bIsChasing);
	DOREPLIFETIME(AGhostAIController, bIsRageEvent);
}