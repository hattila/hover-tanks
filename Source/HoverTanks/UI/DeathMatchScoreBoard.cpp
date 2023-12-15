// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchScoreBoard.h"

#include "DeathMatchPlayerScoreWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/Spacer.h"

UDeathMatchScoreBoard::UDeathMatchScoreBoard(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
		MapName(nullptr),
		GameModeName(nullptr),
		TimeLeftText(nullptr),
		PlayerScoresBox(nullptr),
		TimeLeft(0)
{
	// initialize the player score class
	static ConstructorHelpers::FClassFinder<UUserWidget> PlayerScoreClassFinder(TEXT("/Game/HoverTanks/UI/WBP_DeathMatchPlayerScoreWidget"));
	if (!ensure(PlayerScoreClassFinder.Class != nullptr))
	{
		return;
	}
	PlayerScoreClass = PlayerScoreClassFinder.Class;	
}

bool UDeathMatchScoreBoard::Initialize()
{
	bool bIsSuperInitDone = Super::Initialize();
	if (!bIsSuperInitDone)
	{
		return false;
	}

	return true;
}

void UDeathMatchScoreBoard::Setup()
{
	AddToViewport();
	bIsOpen = true;
}

void UDeathMatchScoreBoard::Teardown()
{
	RemoveFromParent();
	bIsOpen = false;
}

void UDeathMatchScoreBoard::RefreshPlayerScores(const TArray<FDeathMatchPlayerScore>& InPlayerScores)
{
	PlayerScoresArray = InPlayerScores;

	ReDrawPlayerScores();
}

void UDeathMatchScoreBoard::ReDrawPlayerScores()
{
	if (PlayerScoreClass == nullptr)
	{
		return;
	}

	PlayerScoresBox->ClearChildren();

	int32 i = 1;
	for (FDeathMatchPlayerScore PlayerScore : PlayerScoresArray)
	{
		UDeathMatchPlayerScoreWidget* PlayerScoreWidget = CreateWidget<UDeathMatchPlayerScoreWidget>(GetWorld(), PlayerScoreClass);
		if (!PlayerScoreWidget)
		{
			return;
		}
		PlayerScoreWidget->Setup(i, PlayerScore.PlayerName, PlayerScore.Score);
		PlayerScoresBox->AddChild(PlayerScoreWidget);

		// create a Spacer element and Add it to the PlayerScoresBox unless it is the last element
		if (i < PlayerScoresArray.Num())
		{
			USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
			Spacer->SetSize(FVector2d(1, 10));
			PlayerScoresBox->AddChild(Spacer);
		}
		
		++i;
	}
}

