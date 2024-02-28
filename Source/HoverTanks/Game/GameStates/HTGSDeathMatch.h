// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HoverTanks/Game/HTPlayerScore.h"
#include "HTGS_ScoringInterface.h"

#include "CoreMinimal.h"
#include "HTGS_TimerInterface.h"
#include "GameFramework/GameStateBase.h"
#include "HTGSDeathMatch.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API AHTGSDeathMatch :
	public AGameStateBase,
	public IHTGS_ScoringInterface,
	public IHTGS_TimerInterface
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ~IHTGS_ScoringInterface
	virtual void InitializeNewPlayerScore(const APlayerController* NewPlayer) override;
	virtual void RemovePlayersScore(const FString& PlayerName) override;

	virtual TArray<FHTPlayerScore> GetPlayerScores() const override { return PlayerScores; }
	virtual void AddScoreToPlayer(const APlayerController* PlayerController, const int32 ScoreToAdd) override;
	// ~IHTGS_ScoringInterface

	// ~IHTGS_TimerInterface
	virtual int32 GetTimeRemaining() const override { return TimeRemainingInSeconds; }
	virtual void SetTimeRemaining(const int32 NewTimeRemaining) override { TimeRemainingInSeconds = NewTimeRemaining; }
	// ~IHTGS_TimerInterface

protected:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerScores)
	TArray<FHTPlayerScore> PlayerScores;

	UPROPERTY(Replicated)
	int32 TimeRemainingInSeconds;

	UFUNCTION()
	void OnRep_PlayerScores();
	
};
