// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerModule/GameSession/PlayerSessionState.h"
#include "ServerModule/GameSession/GameSessionMode.h"
#include <Net/UnrealNetwork.h>

APlayerSessionState::APlayerSessionState()
{
	bReplicates = true;
}


void APlayerSessionState::RequestSetReady(bool bNewReady)
{
	if (HasAuthority())
	{
		SetReady(bNewReady);
	}
	else
	{
		Server_SetReady(bNewReady);
	}
}

void APlayerSessionState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerSessionState, SeatIndex);
	DOREPLIFETIME(APlayerSessionState, Nickname);
	DOREPLIFETIME(APlayerSessionState, bReady);
	DOREPLIFETIME(APlayerSessionState, bIsEliminated);

	DOREPLIFETIME(APlayerSessionState, LoadState);
}


// Server RPC (레디 상태는 클라가 토글)
void APlayerSessionState::Server_SetReady_Implementation(bool bNewReady)
{
	SetReady(bNewReady);


	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (AGameModeBase* GMBase = World->GetAuthGameMode())
			{
				if (auto* GM = Cast<AGameSessionMode>(GMBase))
				{
					GM->OnPlayerReadyChanged();
				}
			}
		}
	}
}



void APlayerSessionState::SetSeatIndex(int32 NewSeatIndex)
{
	if (!HasAuthority()) return;
	SeatIndex = NewSeatIndex;

	OnRep_SeatIndex();
}

void APlayerSessionState::SetNickname(const FString& NewNickname)
{
	if (!HasAuthority()) return;
	Nickname = NewNickname;

	OnRep_Nickname();
}

void APlayerSessionState::SetReady(bool bNewReady)
{
	if (!HasAuthority()) return;
	bReady = bNewReady;

	OnRep_Ready();
}

void APlayerSessionState::SetEliminated(bool bNewEliminated)
{
	if (!HasAuthority()) return;
	bIsEliminated = bNewEliminated;

	OnRep_Eliminated();
}

void APlayerSessionState::SetLoadState(EPlayerLoadState NewState)
{
	if (!HasAuthority())
		return;

	if (LoadState == NewState)
		return;

	LoadState = NewState;

	OnRep_LoadState();
}

void APlayerSessionState::Server_SetLoadState_Implementation(EPlayerLoadState NewState)
{
	SetLoadState(NewState);

	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (AGameModeBase* GMBase = World->GetAuthGameMode())
			{
				if (auto* GM = Cast<AGameSessionMode>(GMBase))
				{
					GM->OnPlayerLoadStateChanged();
				}
			}
		}
	}
}



// UI 갱신용 코드

void APlayerSessionState::OnRep_SeatIndex()
{

}

void APlayerSessionState::OnRep_Nickname()
{
}

void APlayerSessionState::OnRep_Ready()
{

}

void APlayerSessionState::OnRep_Eliminated()
{
}


void APlayerSessionState::OnRep_LoadState()
{
	BP_OnLoadStateChanged(LoadState);
}