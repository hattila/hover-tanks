// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HTGMDeathMatch.h"
#include "HTGMTeamDeathMatch.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API AHTGMTeamDeathMatch : public AHTGMDeathMatch
{
	GENERATED_BODY()

public:
	AHTGMTeamDeathMatch();

	// ~IHandlesTankDeathGameModeInterface
	virtual void TankDies(AHTHoverTank* DeadHoverTank, AController* DeathCauser) override;
	// ~IHandlesTankDeathGameModeInterface

	// ~ICanRequestRespawnGameModeInterface
	virtual void RequestRespawn(APlayerController* InPlayerController) override;
	// ~ICanRequestRespawnGameModeInterface

protected:
	virtual void BeginPlay() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual void SomeoneKilledSomeone(AController* KillerController, AController* VictimController) override;
};
