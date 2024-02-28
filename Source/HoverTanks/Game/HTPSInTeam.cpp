// Fill out your copyright notice in the Description page of Project Settings.


#include "HTPSInTeam.h"

#include "Net/UnrealNetwork.h"

AHTPSInTeam::AHTPSInTeam()
{
	// OnPawnSet.AddDynamic(this, &AHTPSInTeam::OnPawnSetCallback);
}

void AHTPSInTeam::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHTPSInTeam, MyTeamId);
}

void AHTPSInTeam::SetTeamId(const int8 NewTeamId)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AHTPSInTeam::SetTeamId only callable on the server"));
		return;
	}

	MyTeamId = NewTeamId;
	OnTeamIdChanged.Broadcast(MyTeamId);
}
