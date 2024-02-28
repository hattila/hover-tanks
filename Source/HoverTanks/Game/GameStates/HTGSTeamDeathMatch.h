// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HTGSDeathMatch.h"

#include "CoreMinimal.h"
#include "HTGSTeamDeathMatch.generated.h"

class AInTeamPlayerState;
class UHTTeamDataAsset;
class AHTTeam;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API AHTGSTeamDeathMatch : public AHTGSDeathMatch
{
	GENERATED_BODY()

public:
	AHTGSTeamDeathMatch();

	// ~IScoringGameStateInterface
	virtual void InitializeNewPlayerScore(const APlayerController* NewPlayer) override;
	virtual void AddScoreToPlayer(const APlayerController* PlayerController, const int32 ScoreToAdd) override;
	// ~IScoringGameStateInterface
	
	void CreateTeams();
	void AssignPlayersToTeams();

	bool AssignPlayerToLeastPopulatedTeam(AInTeamPlayerState* PlayerState);
	bool AssignPlayerToTeam(AInTeamPlayerState* TeamPlayerState, int8 TeamId);

	UHTTeamDataAsset* GetTeamDataAsset(const int8 TeamId) const { return TeamsToCreate.FindRef(TeamId); }
	
	TMap<int8, AHTTeam*> GetTeamMap() { return TeamMap; }

	bool AreSameTeam(APlayerController* PlayerController1, APlayerController* PlayerController2);

protected:
	virtual void BeginPlay() override;

	// UFUNCTION()
	// void HandleTeamIdChanged(int8 NewTeamId);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Team", meta = (AllowPrivateAccess = "true"))
	TMap<int8, TObjectPtr<UHTTeamDataAsset>> TeamsToCreate;

	TMap<int8, AHTTeam*> TeamMap;
	
	int8 GetLeastPopulatedTeamId() const;
};
