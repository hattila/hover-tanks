// Fill out your copyright notice in the Description page of Project Settings.


#include "InTeamPlayerState.h"

#include "Net/UnrealNetwork.h"

AInTeamPlayerState::AInTeamPlayerState()
{
	// OnPawnSet.AddDynamic(this, &AInTeamPlayerState::OnPawnSetCallback);
}

void AInTeamPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInTeamPlayerState, MyTeamId);
}

void AInTeamPlayerState::SetTeamId(const int8 NewTeamId)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AInTeamPlayerState::SetTeamId only callable on the server"));
		return;
	}

	MyTeamId = NewTeamId;
	OnTeamIdChanged.Broadcast(MyTeamId);
}
