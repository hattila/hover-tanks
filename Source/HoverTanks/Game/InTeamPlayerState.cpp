// Fill out your copyright notice in the Description page of Project Settings.


#include "InTeamPlayerState.h"

#include "GameModes/TeamDeathMatchGameState.h"
#include "HoverTanks/Pawns/HasTeamColors.h"
#include "Net/UnrealNetwork.h"

AInTeamPlayerState::AInTeamPlayerState()
{
	OnPawnSet.AddDynamic(this, &AInTeamPlayerState::OnPawnSetCallback);
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
	// OnRep_TeamId();
}

void AInTeamPlayerState::OnRep_TeamId() const
{
	// get player controller
	// APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	// OnTeamIdChanged.Broadcast(MyTeamId);
}

void AInTeamPlayerState::OnPawnSetCallback(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
	// return;
	
	// OnTeamIdChanged.Broadcast(MyTeamId);
	// UE_LOG(LogTemp, Warning, TEXT("AInTeamPlayerState::OnPawnSetCallback"));
	
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("AInTeamPlayerState::OnPawnSetCallback no world"));
		return;
	}

	// get hold of the game state
	ATeamDeathMatchGameState* TeamBasedGameState = GetWorld()->GetGameState<ATeamDeathMatchGameState>();
	if (TeamBasedGameState == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AInTeamPlayerState::OnPawnSetCallback no game state"));
		return;
	}

	UTeamDataAsset* TeamDataAsset = TeamBasedGameState->GetTeamDataAsset(MyTeamId);
	if (!TeamDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("AInTeamPlayerState::OnPawnSetCallback no team data asset, my team id is : %d"), MyTeamId);
		return;
	}

	if (IHasTeamColors* TeamColorPawn = Cast<IHasTeamColors>(NewPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("AInTeamPlayerState::OnPawnSetCallback, applying Team Colors"));
		TeamColorPawn->ApplyTeamColors(TeamDataAsset);
	}
}