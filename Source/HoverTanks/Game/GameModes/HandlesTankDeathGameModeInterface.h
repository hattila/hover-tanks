// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HandlesTankDeathGameModeInterface.generated.h"

class AHoverTank;

UINTERFACE()
class UHandlesTankDeathGameModeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IHandlesTankDeathGameModeInterface
{
	GENERATED_BODY()

public:
	virtual void TankDies(AHoverTank* DeadHoverTank, AController* DeathCauser) = 0;
};
