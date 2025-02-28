// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HasTeamColors.generated.h"

class UHTTeamDataAsset;

UINTERFACE()
class UHasTeamColors : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IHasTeamColors
{
	GENERATED_BODY()

public:
	virtual void ApplyTeamColors(UHTTeamDataAsset* TeamDataAsset) = 0;
};
