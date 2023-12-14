// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameState.h"

#include "DeathMatchPlayerScore.h"
#include "GameFramework/PlayerState.h"
#include "HoverTanks/HasScoreBoard.h"
#include "Net/UnrealNetwork.h"

void ADeathMatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// replicate the PlayerScores array
	DOREPLIFETIME(ADeathMatchGameState, PlayerScores);
}

void ADeathMatchGameState::InitializeNewPlayerScore(const APlayerController* NewPlayer)
{
	FDeathMatchPlayerScore PlayerScore;
	PlayerScore.PlayerName = NewPlayer->PlayerState->GetPlayerName();
	PlayerScore.Score = 0;

	PlayerScores.Add(PlayerScore);
}

void ADeathMatchGameState::AddScoreToPlayer(const APlayerController* PlayerController, const int32 ScoreToAdd)
{
	UE_LOG(LogTemp, Warning, TEXT("AddScoreToPlayer, %s, %d"), *PlayerController->PlayerState->GetPlayerName(), ScoreToAdd);
	
	// find the player in PlayerScores and add the score.
	for (int32 i = 0; i < PlayerScores.Num(); i++)
	{
		if (PlayerScores[i].PlayerName == PlayerController->PlayerState->GetPlayerName())
		{
			PlayerScores[i].Score += ScoreToAdd;
			break;
		}
	}

	// sort the scores array by score descending
	PlayerScores.Sort([](const FDeathMatchPlayerScore& PlayerScoreA, const FDeathMatchPlayerScore& PlayerScoreB)
	{
		return PlayerScoreA.Score >= PlayerScoreB.Score;
	});
}

void ADeathMatchGameState::OnRep_PlayerScores()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_PlayerScores happening, num players as know by the  client: %d"), PlayerArray.Num());
	
	// iterate over all connected players and call the OnScoreChange function
	for (int32 i = 0; i < PlayerArray.Num(); i++)
	{
		APlayerController* PlayerController = PlayerArray[i]->GetPlayerController();
		FString PlayerName = PlayerArray[i]->GetPlayerName();
	
		UE_LOG(LogTemp, Warning, TEXT("OnScoresChanged is going to be called on, %s"), *PlayerName);

		// if (PlayerController->GetClass()->ImplementsInterface(UHasScoreBoard::StaticClass())) // crashes
		// if (PlayerController->Implements<UHasScoreBoard>()) // crashes
		if (IHasScoreBoard* ControllerWithScoreBoard = Cast<IHasScoreBoard>(PlayerController))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s implements UHasScoreBoard"), *PlayerName);
			ControllerWithScoreBoard->OnScoresChanged(PlayerScores);

		// 	// PlayerController->OnScoresChanged();
		// 	// IHasScoreBoard::Execute_OnScoreChange(PlayerController, PlayerScores);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s DOES NOT implement UHasScoreBoard"), *PlayerName);
		}
	}
	
}
