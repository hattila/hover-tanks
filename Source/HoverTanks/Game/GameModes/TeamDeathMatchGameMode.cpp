// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamDeathMatchGameMode.h"

#include "TeamDeathMatchGameState.h"
#include "GameFramework/PlayerStart.h"
#include "HoverTanks/HoverTankPlayerController.h"
#include "HoverTanks/Game/InTeamPlayerState.h"
#include "Kismet/GameplayStatics.h"

ATeamDeathMatchGameMode::ATeamDeathMatchGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/HoverTanks/HoverTank/BP_HoverTank"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set the default PlayerController to be HoverTankPlayerController
	PlayerControllerClass = AHoverTankPlayerController::StaticClass();
	
	TArray<AActor*> SpawnPointsInWorld;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPointsInWorld);

	for (AActor* SpawnPoint : SpawnPointsInWorld)
	{
		SpawnPoints.Add(Cast<APlayerStart>(SpawnPoint));
	}

	// set the default PlayerState class to be InTeamPlayerState
	PlayerStateClass = AInTeamPlayerState::StaticClass();

	// set the default game state class to be TeamDeathMatchGameState
	GameStateClass = ATeamDeathMatchGameState::StaticClass();
}

void ATeamDeathMatchGameMode::TankDies(AHoverTank* DeadHoverTank, AController* DeathCauser)
{
	Super::TankDies(DeadHoverTank, DeathCauser);

	UE_LOG(LogTemp, Warning, TEXT("Tank dies in Team Death Match "));
}

void ATeamDeathMatchGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("Team Death Match Begin Play"));

	// create teams
	// sort players to teams

	// get the TeamDeathMatchGameState
	ATeamDeathMatchGameState* TeamDeathMatchGameState = Cast<ATeamDeathMatchGameState>(GameState);
	if (TeamDeathMatchGameState)
	{
		TeamDeathMatchGameState->CreateTeams();
		TeamDeathMatchGameState->AssignPlayersToTeams();
	}
	
}

void ATeamDeathMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AInTeamPlayerState* PlayerState = NewPlayer->GetPlayerState<AInTeamPlayerState>();

	ATeamDeathMatchGameState* TeamDeathMatchGameState = Cast<ATeamDeathMatchGameState>(GameState);
	if (TeamDeathMatchGameState)
	{
		TeamDeathMatchGameState->AssignPlayerToTeam(PlayerState);

		UE_LOG(LogTemp, Warning, TEXT("Player %s assigned to team %d"), *PlayerState->GetPlayerName(), PlayerState->GetTeamId());
	}
}

void ATeamDeathMatchGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}
