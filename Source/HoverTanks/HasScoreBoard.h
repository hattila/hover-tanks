// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/DeathMatchPlayerScore.h"
#include "UObject/Interface.h"
#include "HasScoreBoard.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UHasScoreBoard : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IHasScoreBoard
{
	GENERATED_BODY()

public:

	virtual void OnScoresChanged(TArray<FDeathMatchPlayerScore> PlayerScores) = 0;
};
