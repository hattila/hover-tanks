// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DeathMatchPlayerScore.h"
#include "GameFramework/GameStateBase.h"
#include "DeathMatchGameState.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API ADeathMatchGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	void InitializeNewPlayerScore(const APlayerController* NewPlayer);
	void RemovePlayersScore(const APlayerController* PlayerToRemove);
	
	TArray<FDeathMatchPlayerScore> GetPlayerScores() const { return PlayerScores; }
	void AddScoreToPlayer(const APlayerController* PlayerController, const int32 ScoreToAdd);
	
private:
	TArray<FDeathMatchPlayerScore> PlayerScores;

	UFUNCTION(Server, Unreliable)
	void ServerOnScoreChange();
	
};
