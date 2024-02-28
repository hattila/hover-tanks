// Fill out your copyright notice in the Description page of Project Settings.


#include "HTGMTeamDeathMatch.h"

#include "HoverTanks/Game/GameStates/HTGSTeamDeathMatch.h"
#include "HoverTanks/Game/HTPSInTeam.h"
#include "HoverTanks/Game/Teams/HTTeamDataAsset.h"
#include "HoverTanks/Pawns/HoverTank/HTHoverTank.h"
#include "HoverTanks/Controllers/HTPlayerController.h"
#include "HoverTanks/UI/HUD/HTTeamDeathMatchHUD.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AHTGMTeamDeathMatch::AHTGMTeamDeathMatch()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/HoverTanks/Pawns/HoverTank/BP_HoverTank"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PlayerControllerClass = AHTPlayerController::StaticClass();
	
	TArray<AActor*> SpawnPointsInWorld;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPointsInWorld);
	for (AActor* SpawnPoint : SpawnPointsInWorld)
	{
		SpawnPoints.Add(Cast<APlayerStart>(SpawnPoint));
	}

	PlayerStateClass = AHTPSInTeam::StaticClass();
	GameStateClass = AHTGSTeamDeathMatch::StaticClass();
	HUDClass = AHTTeamDeathMatchHUD::StaticClass();
}

void AHTGMTeamDeathMatch::TankDies(AHTHoverTank* DeadHoverTank, AController* DeathCauser)
{
	// UE_LOG(LogTemp, Warning, TEXT("Tank %s died in TDM!"), *DeadHoverTank->GetName());

	APlayerController* DeadPlayerController = Cast<APlayerController>(DeadHoverTank->GetController());
	if (DeadPlayerController)
	{
		SomeoneKilledSomeone(DeathCauser,DeadPlayerController);

		APlayerController* KillerPlayerController = Cast<APlayerController>(DeathCauser);
		
		AHTGSTeamDeathMatch* TeamGameState = GetGameState<AHTGSTeamDeathMatch>();
		if (TeamGameState && KillerPlayerController != nullptr)
		{
			const int32 ScoreToAdd = TeamGameState->AreSameTeam(DeadPlayerController, KillerPlayerController) ? -1 : 1;
			TeamGameState->AddScoreToPlayer(KillerPlayerController, ScoreToAdd);
		}

		DeadHoverTank->OnDeath();
	}
	
}

void AHTGMTeamDeathMatch::RequestRespawn(APlayerController* InPlayerController)
{
	AHTGSTeamDeathMatch* TeamDeathMatchGameState = GetGameState<AHTGSTeamDeathMatch>();

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
				UE_LOG(LogTemp, Warning, TEXT("AHTDeathMatchGameMode::RequestRespawn - CurrentPawn is null"));
			}

			APlayerStart* RandomSpawnPoint = FindRandomSpawnPoint();
			AHTHoverTank* NewHoverTank = SpawnTankAtPlayerStart(RandomSpawnPoint);

			const AHTPSInTeam* TeamPlayerState = InPlayerController->GetPlayerState<AHTPSInTeam>();
			IHasTeamColors* TeamColoredPawn = Cast<IHasTeamColors>(NewHoverTank);
			if (TeamPlayerState && TeamColoredPawn)
			{
				UHTTeamDataAsset* TeamDataAsset = TeamDeathMatchGameState->GetTeamDataAsset(TeamPlayerState->GetTeamId());
				TeamColoredPawn->ApplyTeamColors(TeamDataAsset);
			}
			
			InPlayerController->Possess(NewHoverTank);
		}
	}
}

void AHTGMTeamDeathMatch::BeginPlay()
{
	Super::BeginPlay();

	AHTGSTeamDeathMatch* TeamGameState = Cast<AHTGSTeamDeathMatch>(GameState);
	if (TeamGameState)
	{
		TeamGameState->CreateTeams();
		TeamGameState->AssignPlayersToTeams();
	}
	
}

void AHTGMTeamDeathMatch::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AHTPSInTeam* PlayerState = NewPlayer->GetPlayerState<AHTPSInTeam>();

	AHTGSTeamDeathMatch* TeamGameState = Cast<AHTGSTeamDeathMatch>(GameState);
	if (TeamGameState)
	{
		const bool bIsAssigned = TeamGameState->AssignPlayerToLeastPopulatedTeam(PlayerState);
		// FString IsAssigned = bIsAssigned ? TEXT("") : TEXT("NOT");
		// UE_LOG(LogTemp, Warning, TEXT("Player %s was %s assigned to team %d"), *PlayerState->GetPlayerName(), *IsAssigned, PlayerState->GetTeamId());
	}
}

void AHTGMTeamDeathMatch::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void AHTGMTeamDeathMatch::SomeoneKilledSomeone(AController* KillerController, AController* VictimController)
{
	const FString KillerName = KillerController->PlayerState->GetPlayerName();
	const FString VictimName = VictimController->PlayerState->GetPlayerName();

	FLinearColor KillerTeamColor;
	FLinearColor VictimTeamColor;
	
	// find the team colors
	const AHTPSInTeam* KillerTeamPlayerState = Cast<AHTPSInTeam>(KillerController->PlayerState);
	const AHTPSInTeam* VictimTeamPlayerState = Cast<AHTPSInTeam>(VictimController->PlayerState);
	const AHTGSTeamDeathMatch* TeamDeathMatchGameState = GetGameState<AHTGSTeamDeathMatch>();

	if (KillerTeamPlayerState && VictimTeamPlayerState && TeamDeathMatchGameState)
	{
		KillerTeamColor = TeamDeathMatchGameState->GetTeamDataAsset(KillerTeamPlayerState->GetTeamId())->GetTextColor();
		VictimTeamColor = TeamDeathMatchGameState->GetTeamDataAsset(VictimTeamPlayerState->GetTeamId())->GetTextColor();	
	}

	// iterate over every joined player, cast their PlayerControllers to AHTPlayerController and call ClientAddKillIndicator
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHTPlayerController* PlayerController = Cast<AHTPlayerController>(It->Get());
		if (PlayerController)
		{
			PlayerController->ClientAddKillIndicator(KillerName, VictimName, KillerTeamColor, VictimTeamColor);
		}
	}
}
