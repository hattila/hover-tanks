// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HoverTanks/Game/PlayerScore.h"

#include "CoreMinimal.h"
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
	
	virtual void InitializeNewPlayerScore(const APlayerController* NewPlayer);
	void RemovePlayersScore(const FString& PlayerName);
	
	TArray<FPlayerScore> GetPlayerScores() const { return PlayerScores; }
	virtual void AddScoreToPlayer(const APlayerController* PlayerController, const int32 ScoreToAdd);

	int32 GetTimeRemaining() const { return TimeRemainingInSeconds; }
	void SetTimeRemaining(const int32 NewTimeRemaining) { TimeRemainingInSeconds = NewTimeRemaining; }
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerScores)
	TArray<FPlayerScore> PlayerScores;

	UPROPERTY(Replicated)
	int32 TimeRemainingInSeconds;

	UFUNCTION()
	void OnRep_PlayerScores();
	
};