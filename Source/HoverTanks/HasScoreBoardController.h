// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HasScoreBoardController.generated.h"

UINTERFACE()
class UHasScoreBoardController : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IHasScoreBoardController
{
	GENERATED_BODY()

public:
	UFUNCTION(Client, Unreliable)
	virtual void ClientOnScoresChanged() = 0;

	UFUNCTION(Client, Reliable)
	virtual void ClientForceOpenScoreBoard(int32 TimeUntilRestartInSeconds) = 0;
};
