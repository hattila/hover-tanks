// Fill out your copyright notice in the Description page of Project Settings.


#include "InTeamPlayerState.h"

#include "Net/UnrealNetwork.h"

void AInTeamPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInTeamPlayerState, MyTeamId);
}
