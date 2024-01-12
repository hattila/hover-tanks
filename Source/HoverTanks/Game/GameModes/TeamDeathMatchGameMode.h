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

	// ~IHandlesTankDeathGameModeInterface
	virtual void TankDies(AHoverTank* DeadHoverTank, AController* DeathCauser) override;
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
