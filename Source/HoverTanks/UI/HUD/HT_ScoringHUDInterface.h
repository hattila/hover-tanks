// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/HUD.h" // do not remove this!
#include "HT_ScoringHUDInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UHT_ScoringHUDInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IHT_ScoringHUDInterface
{
	GENERATED_BODY()

public:
	virtual void ToggleScoreBoard() = 0;
	virtual void ForceOpenScoreBoard() = 0;
	virtual void RefreshPlayerScores() = 0;

	// separate interface?
	virtual void AddKillIndicator(
		const FString& KillerName,
		const FString& VictimName,
		FLinearColor KillerColor = FLinearColor(1,1,1,1),
		FLinearColor VictimColor = FLinearColor(1,1,1,1)
	) = 0;
};
