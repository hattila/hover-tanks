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

	virtual void TankDies(AHoverTank* DeadHoverTank);

protected:
	TArray<APlayerStart*> SpawnPoints;
	
	virtual void BeginPlay() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	// todo : on logout
	// virtual void Logout(AController* Exiting) override;
	
	
	APlayerStart* FindRandomSpawnPoint();
	AHoverTank* SpawnTankAtPlayerStart(APlayerStart* RandomSpawnPoint);
	
private:
	
};
