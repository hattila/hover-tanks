// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DeathMatchGameMode.generated.h"

class AHoverTank;
class APlayerStart;

/**
 * 
 */
UCLASS()
class HOVERTANKS_API ADeathMatchGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADeathMatchGameMode();

	void TankDies(AHoverTank* DeadHoverTank);

protected:
	virtual void BeginPlay() override;

private:
	TArray<APlayerStart*> SpawnPoints;

	APlayerStart* FindRandomSpawnPoint();
	AHoverTank* SpawnTankAtPlayerStart(APlayerStart* RandomSpawnPoint);
};
