// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/HUD.h" // do not remove this!
#include "ScoringHUDInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UScoringHUDInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IScoringHUDInterface
{
	GENERATED_BODY()

public:
	virtual void ToggleScoreBoard() = 0;
	virtual void ForceOpenScoreBoard() = 0;
	virtual void RefreshPlayerScores() = 0;
};
