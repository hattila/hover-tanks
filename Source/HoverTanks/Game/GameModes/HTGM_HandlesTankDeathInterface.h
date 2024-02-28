// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/GameModeBase.h"
#include "HTGM_HandlesTankDeathInterface.generated.h"

class AHoverTank;

UINTERFACE()
class UHTGM_HandlesTankDeathInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IHTGM_HandlesTankDeathInterface
{
	GENERATED_BODY()

public:
	virtual void TankDies(AHoverTank* DeadHoverTank, AController* DeathCauser) = 0;
};
