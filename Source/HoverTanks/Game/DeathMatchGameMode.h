// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DeathMatchGameMode.generated.h"

class AHoverTank;
class APlayerStart;

UENUM()
enum EMatchState
{
	// add states like WaitingToStart, InProgress, GameOver
	WaitingToStart,
	InProgress,
	GameOver
};

/**
 * 
 */
UCLASS()
class HOVERTANKS_API ADeathMatchGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADeathMatchGameMode();

	virtual void TankDies(AHoverTank* DeadHoverTank, AController* DeathCauser);
	void RequestRespawn(APlayerController* InPlayerController);

protected:
	TArray<APlayerStart*> SpawnPoints;

	int32 MatchTimeInSeconds = 30;

	EMatchState MatchState = EMatchState::InProgress; // todo change to WaitingToStart
	
	virtual void BeginPlay() override;
	void OnOneSecondElapsed();

	void GameOver();
	virtual void ResetLevel() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	
	
	APlayerStart* FindRandomSpawnPoint();
	AHoverTank* SpawnTankAtPlayerStart(APlayerStart* RandomSpawnPoint);
	
private:
	FTimerHandle GameTimerHandle;
	
};
