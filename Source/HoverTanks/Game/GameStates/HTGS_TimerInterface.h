// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HTGS_TimerInterface.generated.h"

UINTERFACE()
class UHTGS_TimerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IHTGS_TimerInterface
{
	GENERATED_BODY()

public:
	virtual int32 GetTimeRemaining() const = 0;
	virtual void SetTimeRemaining(const int32 NewTimeRemaining) = 0;
};
