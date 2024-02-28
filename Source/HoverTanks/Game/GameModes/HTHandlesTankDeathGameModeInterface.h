// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/GameModeBase.h"
#include "HTHandlesTankDeathGameModeInterface.generated.h"

class AHoverTank;

UINTERFACE()
class UHTHandlesTankDeathGameModeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IHTHandlesTankDeathGameModeInterface
{
	GENERATED_BODY()

public:
	virtual void TankDies(AHoverTank* DeadHoverTank, AController* DeathCauser) = 0;
};
