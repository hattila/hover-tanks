// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CanRequestRespawnGameModeInterface.h"
#include "GameFramework/GameModeBase.h"
#include "DeathMatchGameMode.generated.h"

class AHoverTank;
class APlayerStart;

UENUM()
enum EMatchState
{
	WaitingToStart,
	InProgress,
	GameOver
};

/**
 * 
 */
UCLASS()
class HOVERTANKS_API ADeathMatchGameMode : public AGameModeBase, public ICanRequestRespawnGameModeInterface
{
	GENERATED_BODY()

public:
	ADeathMatchGameMode();

	virtual void TankDies(AHoverTank* DeadHoverTank, AController* DeathCauser);

	// ~ICanRequestRespawnGameModeInterface
	virtual void RequestRespawn(APlayerController* InPlayerController) override;
	// ~ICanRequestRespawnGameModeInterface

protected:
	TArray<APlayerStart*> SpawnPoints;

	int32 MatchTimeInSeconds = 180;

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

	FTimerHandle OnLogoutScoreRefreshTimerHandle;
	void RemovePlayerFromScoreBoardOnLogout(const FString PlayerName);
	
};
