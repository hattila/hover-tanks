// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ScoringGameStateInterface.generated.h"

struct FPlayerScore;

UINTERFACE()
class UScoringGameStateInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IScoringGameStateInterface
{
	GENERATED_BODY()

public:
	virtual void InitializeNewPlayerScore(const APlayerController* NewPlayer) = 0;
	virtual void RemovePlayersScore(const FString& PlayerName) = 0;
	
	virtual TArray<FPlayerScore> GetPlayerScores() const = 0;
	virtual void AddScoreToPlayer(const APlayerController* PlayerController, const int32 ScoreToAdd) = 0;
};
