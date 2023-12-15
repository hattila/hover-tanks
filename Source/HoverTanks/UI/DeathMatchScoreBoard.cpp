// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchScoreBoard.h"

#include "DeathMatchPlayerScoreWidget.h"
#include "Components/ScrollBox.h"

UDeathMatchScoreBoard::UDeathMatchScoreBoard(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
                                                                                            MapName(nullptr),
                                                                                            GameModeName(nullptr),
                                                                                            TimeLeftText(nullptr),
                                                                                            PlayerScoresBox(nullptr),
                                                                                            TimeLeft(0)
{
	// initialize the player score class
	static ConstructorHelpers::FClassFinder<UUserWidget> PlayerScoreClassFinder(TEXT("/Game/HoverTanks/UI/WBP_DeathMatchPlayerScoreWidget"));
	if (!PlayerScoreClass)
	{
		return;
	}
	PlayerScoreClass = PlayerScoreClassFinder.Class;
}

void UDeathMatchScoreBoard::Setup()
{
	AddToViewport();
}

void UDeathMatchScoreBoard::Teardown()
{
	RemoveFromParent();
}

void UDeathMatchScoreBoard::RefreshPlayerScores(const TArray<FDeathMatchPlayerScore> InPlayerScores)
{
	PlayerScoresArray = InPlayerScores;

	ReDrawPlayerScores();
}

void UDeathMatchScoreBoard::ReDrawPlayerScores()
{
	PlayerScoresBox->ClearChildren();

	// for each entry in PlayerScoresArray create a playerScore widget and add it to the PlayerScoresBox
	int32 i = 1;
	for (FDeathMatchPlayerScore PlayerScore : PlayerScoresArray)
	{
		UDeathMatchPlayerScoreWidget* PlayerScoreWidget = CreateWidget<UDeathMatchPlayerScoreWidget>(GetWorld(), PlayerScoreClass);
		if (!PlayerScoreWidget)
		{
			return;
		}
		PlayerScoreWidget->Setup(i, PlayerScore.PlayerName, PlayerScore.Score);
		++i;
		
		PlayerScoresBox->AddChild(PlayerScoreWidget);
	}
}

