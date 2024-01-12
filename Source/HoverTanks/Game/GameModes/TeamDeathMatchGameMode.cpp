// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamDeathMatchGameMode.h"

#include "HoverTanks/Game/GameStates/TeamDeathMatchGameState.h"
#include "HoverTanks/Pawns/HoverTank.h"
#include "HoverTanks/Controllers/HoverTankPlayerController.h"
#include "HoverTanks/Game/InTeamPlayerState.h"
#include "HoverTanks/UI/HUD/TeamDeathMatchHUD.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ATeamDeathMatchGameMode::ATeamDeathMatchGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/HoverTanks/Pawns/BP_HoverTank"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PlayerControllerClass = AHoverTankPlayerController::StaticClass();
	
	TArray<AActor*> SpawnPointsInWorld;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPointsInWorld);
	for (AActor* SpawnPoint : SpawnPointsInWorld)
	{
		SpawnPoints.Add(Cast<APlayerStart>(SpawnPoint));
	}

	PlayerStateClass = AInTeamPlayerState::StaticClass();
	GameStateClass = ATeamDeathMatchGameState::StaticClass();
	HUDClass = ATeamDeathMatchHUD::StaticClass();
}

void ATeamDeathMatchGameMode::TankDies(AHoverTank* DeadHoverTank, AController* DeathCauser)
{
	// UE_LOG(LogTemp, Warning, TEXT("Tank %s died in TDM!"), *DeadHoverTank->GetName());

	APlayerController* DeadPlayerController = Cast<APlayerController>(DeadHoverTank->GetController());
	if (DeadPlayerController)
	{
		// Kill indicator
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.f,
			FColor::Green,
			FString::Printf(
				TEXT("%s died!"),
				*DeadPlayerController->PlayerState->GetPlayerName()
			)
		);

		APlayerController* KillerPlayerController = Cast<APlayerController>(DeathCauser);
		
		ATeamDeathMatchGameState* TeamGameState = GetGameState<ATeamDeathMatchGameState>();
		if (TeamGameState && KillerPlayerController != nullptr)
		{
			const int32 ScoreToAdd = TeamGameState->AreSameTeam(DeadPlayerController, KillerPlayerController) ? -1 : 1;
			TeamGameState->AddScoreToPlayer(KillerPlayerController, ScoreToAdd);
		}

		DeadHoverTank->OnDeath();
	}
	
}

void ATeamDeathMatchGameMode::RequestRespawn(APlayerController* InPlayerController)
{
	ATeamDeathMatchGameState* TeamDeathMatchGameState = GetGameState<ATeamDeathMatchGameState>();

	if (TeamDeathMatchGameState)
	{
		const int32 TimeRemaining = TeamDeathMatchGameState->GetTimeRemaining();
		if (TimeRemaining > 0 && MatchState == EMatchState::InProgress)
		{
			APawn* CurrentPawn = InPlayerController->GetPawn();
			InPlayerController->UnPossess();
			if (CurrentPawn)
			{
				CurrentPawn->Destroy();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ADeathMatchGameMode::RequestRespawn - CurrentPawn is null"));
			}

			APlayerStart* RandomSpawnPoint = FindRandomSpawnPoint();
			AHoverTank* NewHoverTank = SpawnTankAtPlayerStart(RandomSpawnPoint);

			const AInTeamPlayerState* TeamPlayerState = InPlayerController->GetPlayerState<AInTeamPlayerState>();
			IHasTeamColors* TeamColoredPawn = Cast<IHasTeamColors>(NewHoverTank);
			if (TeamPlayerState && TeamColoredPawn)
			{
				UTeamDataAsset* TeamDataAsset = TeamDeathMatchGameState->GetTeamDataAsset(TeamPlayerState->GetTeamId());
				TeamColoredPawn->ApplyTeamColors(TeamDataAsset);
			}
			
			InPlayerController->Possess(NewHoverTank);
		}
	}
}

void ATeamDeathMatchGameMode::BeginPlay()
{
	Super::BeginPlay();

	ATeamDeathMatchGameState* TeamGameState = Cast<ATeamDeathMatchGameState>(GameState);
	if (TeamGameState)
	{
		TeamGameState->CreateTeams();
		TeamGameState->AssignPlayersToTeams();
	}
	
}

void ATeamDeathMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AInTeamPlayerState* PlayerState = NewPlayer->GetPlayerState<AInTeamPlayerState>();

	ATeamDeathMatchGameState* TeamGameState = Cast<ATeamDeathMatchGameState>(GameState);
	if (TeamGameState)
	{
		const bool bIsAssigned = TeamGameState->AssignPlayerToLeastPopulatedTeam(PlayerState);
		// FString IsAssigned = bIsAssigned ? TEXT("") : TEXT("NOT");
		// UE_LOG(LogTemp, Warning, TEXT("Player %s was %s assigned to team %d"), *PlayerState->GetPlayerName(), *IsAssigned, PlayerState->GetTeamId());
	}
}

void ATeamDeathMatchGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

