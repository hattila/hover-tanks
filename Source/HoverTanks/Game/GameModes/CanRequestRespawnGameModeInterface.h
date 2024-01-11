// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CanRequestRespawnGameModeInterface.generated.h"

UINTERFACE()
class UCanRequestRespawnGameModeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API ICanRequestRespawnGameModeInterface
{
	GENERATED_BODY()

public:
	virtual void RequestRespawn(APlayerController* InPlayerController) = 0;
};
