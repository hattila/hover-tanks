// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HTGM_CanRequestRespawnInterface.h"
#include "HTGM_HandlesTankDeathInterface.h"
#include "GameFramework/GameModeBase.h"
#include "HTGMDeathMatch.generated.h"

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
class HOVERTANKS_API AHTGMDeathMatch :
	public AGameModeBase,
	public IHTGM_CanRequestRespawnInterface,
	public IHTGM_HandlesTankDeathInterface
{
	GENERATED_BODY()

public:
	AHTGMDeathMatch();

	// ~IHTGMHandlesTankDeathInterface
	virtual void TankDies(AHoverTank* DeadHoverTank, AController* DeathCauser) override;
	// ~IHTGMHandlesTankDeathInterface

	// ~IHTCanRequestRespawnInterface
	virtual void RequestRespawn(APlayerController* InPlayerController) override;
	// ~IHTCanRequestRespawnInterface

protected:
	TArray<APlayerStart*> SpawnPoints;

	int32 MatchTimeInSeconds = 180;

	EMatchState MatchState = EMatchState::InProgress;
	
	virtual void BeginPlay() override;
	virtual void OnOneSecondElapsed();
	virtual void GameOver();
	virtual void ResetLevel() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	
	APlayerStart* FindRandomSpawnPoint();
	AHoverTank* SpawnTankAtPlayerStart(APlayerStart* RandomSpawnPoint);

	virtual void RemovePlayerFromScoreBoardOnLogout(const FString PlayerName);

	virtual void SomeoneKilledSomeone(AController* KillerController, AController* VictimController);

private:
	FTimerHandle GameTimerHandle;
	FTimerHandle OnLogoutScoreRefreshTimerHandle;

};
