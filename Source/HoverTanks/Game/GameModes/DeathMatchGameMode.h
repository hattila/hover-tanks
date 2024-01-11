// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CanRequestRespawnGameModeInterface.h"
#include "HandlesTankDeathGameModeInterface.h"
#include "GameFramework/GameModeBase.h"
#include "DeathMatchGameMode.generated.h"

class AHoverTank;
class APlayerStart;

UENUM()
enum EMatchState
{
	WaitingToStart, // soon tm
	InProgress,
	GameOver
};

/**
 * 
 */
UCLASS()
class HOVERTANKS_API ADeathMatchGameMode :
	public AGameModeBase,
	public ICanRequestRespawnGameModeInterface,
	public IHandlesTankDeathGameModeInterface
{
	GENERATED_BODY()

public:
	ADeathMatchGameMode();

	// ~IHandlesTankDeathGameModeInterface
	virtual void TankDies(AHoverTank* DeadHoverTank, AController* DeathCauser) override;
	// ~IHandlesTankDeathGameModeInterface

	// ~ICanRequestRespawnGameModeInterface
	virtual void RequestRespawn(APlayerController* InPlayerController) override;
	// ~ICanRequestRespawnGameModeInterface

protected:
	TArray<APlayerStart*> SpawnPoints;

	int32 MatchTimeInSeconds = 180;

	EMatchState MatchState = EMatchState::InProgress;
	
	virtual void BeginPlay() override;
	void OnOneSecondElapsed();
	void GameOver();
	virtual void ResetLevel() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	
	APlayerStart* FindRandomSpawnPoint();
	AHoverTank* SpawnTankAtPlayerStart(APlayerStart* RandomSpawnPoint);

	void RemovePlayerFromScoreBoardOnLogout(const FString PlayerName);

private:
	FTimerHandle GameTimerHandle;
	FTimerHandle OnLogoutScoreRefreshTimerHandle;

};
