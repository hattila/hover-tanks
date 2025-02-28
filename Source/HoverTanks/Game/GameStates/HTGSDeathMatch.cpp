// Fill out your copyright notice in the Description page of Project Settings.


#include "HTGSDeathMatch.h"

#include "HoverTanks/Game/HTPlayerScore.h"
#include "HoverTanks/Controllers/HTHasScoreBoardInterface.h"

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

void AHTGSDeathMatch::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHTGSDeathMatch, TimeRemainingInSeconds);
	DOREPLIFETIME(AHTGSDeathMatch, PlayerScores);
}

void AHTGSDeathMatch::InitializeNewPlayerScore(const APlayerController* NewPlayer)
{
	// return if the player is already in the array
	for (int32 i = 0; i < PlayerScores.Num(); i++)
	{
		if (PlayerScores[i].PlayerName == NewPlayer->PlayerState->GetPlayerName())
		{
			return;
		}
	}
	
	FHTPlayerScore PlayerScore;
	PlayerScore.PlayerName = NewPlayer->PlayerState->GetPlayerName();
	PlayerScore.Score = 0;

	PlayerScores.Add(PlayerScore);

	OnRep_PlayerScores();
}

void AHTGSDeathMatch::RemovePlayersScore(const FString& PlayerName)
{
	// find the Player by name and remove it from the array
	for (int32 i = 0; i < PlayerScores.Num(); i++)
	{
		if (PlayerScores[i].PlayerName == PlayerName)
		{
			PlayerScores.RemoveAt(i);
			break;
		}
	}

	OnRep_PlayerScores();
}

void AHTGSDeathMatch::AddScoreToPlayer(const APlayerController* PlayerController, const int32 ScoreToAdd)
{
	// UE_LOG(LogTemp, Warning, TEXT("AddScoreToPlayer, %s, %d"), *PlayerController->PlayerState->GetPlayerName(), ScoreToAdd);
	
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
	PlayerScores.Sort([](const FHTPlayerScore& PlayerScoreA, const FHTPlayerScore& PlayerScoreB)
	{
		return PlayerScoreA.Score >= PlayerScoreB.Score;
	});

	OnRep_PlayerScores();
}

void AHTGSDeathMatch::OnRep_PlayerScores()
{
	for (int32 i = 0; i < PlayerArray.Num(); i++)
	{
		APlayerController* PlayerController = PlayerArray[i]->GetPlayerController();
		FString PlayerName = PlayerArray[i]->GetPlayerName();
		
		// UE_LOG(LogTemp, Warning, TEXT("OnScoresChanged is going to be called on, %s"), *PlayerName);

		if (IHTHasScoreBoardInterface* ControllerWithScoreBoard = Cast<IHTHasScoreBoardInterface>(PlayerController))
		{
			ControllerWithScoreBoard->ClientOnScoresChanged();
		}
	}
}