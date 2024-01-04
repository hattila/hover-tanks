// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HoverTanks/Game/PlayerScore.h"
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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void InitializeNewPlayerScore(const APlayerController* NewPlayer);
	void RemovePlayersScore(const FString& PlayerName);
	
	TArray<FPlayerScore> GetPlayerScores() const { return PlayerScores; }
	void AddScoreToPlayer(const APlayerController* PlayerController, const int32 ScoreToAdd);

	int32 GetTimeRemaining() const { return TimeRemainingInSeconds; }
	void SetTimeRemaining(const int32 NewTimeRemaining) { TimeRemainingInSeconds = NewTimeRemaining; }
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerScores)
	TArray<FPlayerScore> PlayerScores;

	UPROPERTY(Replicated)
	int32 TimeRemainingInSeconds;

	UFUNCTION()
	void OnRep_PlayerScores();
	
};
