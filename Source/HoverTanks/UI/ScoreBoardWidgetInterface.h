// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ScoreBoardWidgetInterface.generated.h"

struct FDeathMatchPlayerScore;
class UTextBlock;
// This class does not need to be modified.
UINTERFACE()
class UScoreBoardWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IScoreBoardWidgetInterface
{
	GENERATED_BODY()

public:
	virtual void Setup() = 0;
	virtual void Teardown() = 0;

	virtual void SetMapName(const FString& InMapName) const = 0;
	virtual void SetGameModeName(const FString& InGameModeName) const = 0;
	virtual void SetTimeLeft(int32 InTimeLeft) = 0;
	
	virtual void RefreshTimeLeft() = 0;

	// virtual void RefreshPlayerScores(const TArray<FDeathMatchPlayerScore>& InPlayerScores) = 0; // ! PlayerScoreInterface!
};
