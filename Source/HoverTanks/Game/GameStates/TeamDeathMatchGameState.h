// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DeathMatchGameState.h"

#include "CoreMinimal.h"
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

	// ~IScoringGameStateInterface
	virtual void InitializeNewPlayerScore(const APlayerController* NewPlayer) override;
	virtual void AddScoreToPlayer(const APlayerController* PlayerController, const int32 ScoreToAdd) override;
	// ~IScoringGameStateInterface
	
	void CreateTeams();
	void AssignPlayersToTeams();

	bool AssignPlayerToLeastPopulatedTeam(AInTeamPlayerState* PlayerState);
	bool AssignPlayerToTeam(AInTeamPlayerState* TeamPlayerState, int8 TeamId);

	UTeamDataAsset* GetTeamDataAsset(const int8 TeamId) const { return TeamsToCreate.FindRef(TeamId); }
	
	TMap<int8, ATeam*> GetTeamMap() { return TeamMap; }

	bool AreSameTeam(APlayerController* PlayerController1, APlayerController* PlayerController2);

protected:
	virtual void BeginPlay() override;

	// UFUNCTION()
	// void HandleTeamIdChanged(int8 NewTeamId);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Team", meta = (AllowPrivateAccess = "true"))
	TMap<int8, TObjectPtr<UTeamDataAsset>> TeamsToCreate;

	TMap<int8, ATeam*> TeamMap;
	
	int8 GetLeastPopulatedTeamId() const;
};
