// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HoverTanks/Game/PlayerScore.h"
#include "ScoringGameStateInterface.h"

#include "CoreMinimal.h"
#include "TimerGameStateInterface.h"
#include "GameFramework/GameStateBase.h"
#include "DeathMatchGameState.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API ADeathMatchGameState :
	public AGameStateBase,
	public IScoringGameStateInterface,
	public ITimerGameStateInterface
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ~IScoringGameStateInterface
	virtual void InitializeNewPlayerScore(const APlayerController* NewPlayer) override;
	virtual void RemovePlayersScore(const FString& PlayerName) override;

	virtual TArray<FPlayerScore> GetPlayerScores() const override { return PlayerScores; }
	virtual void AddScoreToPlayer(const APlayerController* PlayerController, const int32 ScoreToAdd) override;
	// ~IScoringGameStateInterface

	// ~ITimerGameStateInterface
	virtual int32 GetTimeRemaining() const override { return TimeRemainingInSeconds; }
	virtual void SetTimeRemaining(const int32 NewTimeRemaining) override { TimeRemainingInSeconds = NewTimeRemaining; }
	// ~ITimerGameStateInterface

protected:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerScores)
	TArray<FPlayerScore> PlayerScores;

	UPROPERTY(Replicated)
	int32 TimeRemainingInSeconds;

	UFUNCTION()
	void OnRep_PlayerScores();
	
};
