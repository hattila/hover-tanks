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
	// get replication method
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeNewPlayerScore(const APlayerController* NewPlayer);
	TArray<FDeathMatchPlayerScore> GetPlayerScores() const { return PlayerScores; }
	void AddScoreToPlayer(const APlayerController* PlayerController, const int32 ScoreToAdd);
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerScores)
	TArray<FDeathMatchPlayerScore> PlayerScores;

	UFUNCTION()
	void OnRep_PlayerScores();
	
};
