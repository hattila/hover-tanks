// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HTGM_CanRequestRespawnInterface.generated.h"

UINTERFACE()
class UHTGM_CanRequestRespawnInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IHTGM_CanRequestRespawnInterface
{
	GENERATED_BODY()

public:
	virtual void RequestRespawn(APlayerController* InPlayerController) = 0;
};
