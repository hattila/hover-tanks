// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DeathMatchGameMode.h"
#include "TeamDeathMatchGameMode.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API ATeamDeathMatchGameMode : public ADeathMatchGameMode
{
	GENERATED_BODY()

public:
	ATeamDeathMatchGameMode();

	virtual void TankDies(AHoverTank* DeadHoverTank);

protected:
	virtual void BeginPlay() override;
};
