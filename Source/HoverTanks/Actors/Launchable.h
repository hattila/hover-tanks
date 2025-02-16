// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Launchable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class ULaunchable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API ILaunchable
{
	GENERATED_BODY()

public:
	virtual void DirectionalLaunch(const FVector& LaunchVelocity) = 0;
};
