// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DeathMatchGameState.h"
#include "TeamDeathMatchGameState.generated.h"

class AInTeamPlayerState;
class UTeamDataAsset;
class ATeam;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API ATeamDeathMatchGameState : public ADeathMatchGameState
{
	GENERATED_BODY()

public:
	ATeamDeathMatchGameState();
	
	void CreateTeams();
	void AssignPlayersToTeams();

	bool AssignPlayerToLeastPopulatedTeam(AInTeamPlayerState* PlayerState);
	bool AssignPlayerToTeam(AInTeamPlayerState* TeamPlayerState, int8 TeamId);

	UTeamDataAsset* GetTeamDataAsset(const int8 TeamId) const { return TeamsToCreate.FindRef(TeamId); }
	
	TMap<int8, ATeam*> GetTeamMap() { return TeamMap; }
private:
	UPROPERTY(EditDefaultsOnly, Category = "Team", meta = (AllowPrivateAccess = "true"))
	TMap<int8, TObjectPtr<UTeamDataAsset>> TeamsToCreate;

	TMap<int8, ATeam*> TeamMap;
	
	int8 GetLeastPopulatedTeamId() const;
};
