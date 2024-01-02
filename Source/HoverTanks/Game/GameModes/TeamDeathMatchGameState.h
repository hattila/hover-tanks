// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TeamDeathMatchGameState.generated.h"

class AInTeamPlayerState;
class UTeamDataAsset;
class ATeam;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API ATeamDeathMatchGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ATeamDeathMatchGameState();
	
	void CreateTeams();
	void AssignPlayersToTeams();
	void AssignPlayerToTeam(AInTeamPlayerState* PlayerState);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Team", meta = (AllowPrivateAccess = "true"))
	TMap<int8, TObjectPtr<UTeamDataAsset>> TeamsToCreate;

	TMap<int8, ATeam*> TeamMap;
	
	int8 GetLeastPopulatedTeamId() const;
};
