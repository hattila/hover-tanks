// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchScoreBoardWidget.h"

#include "Components/ScrollBox.h"
#include "PlayerScoreWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Spacer.h"

UDeathMatchScoreBoardWidget::UDeathMatchScoreBoardWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// initialize the player score class
	static ConstructorHelpers::FClassFinder<UUserWidget> PlayerScoreClassFinder(
		TEXT("/Game/HoverTanks/UI/Scoreboard/WBP_PlayerScoreWidget"));
	if (!ensure(PlayerScoreClassFinder.Class != nullptr))
	{
		return;
	}
	PlayerScoreClass = PlayerScoreClassFinder.Class;
}

void UDeathMatchScoreBoardWidget::RefreshPlayerScores(const TArray<FPlayerScore>& InPlayerScores)
{
	if (PlayerScoreClass == nullptr || PlayerScoresBox == nullptr)
	{
		return;
	}

	PlayerScoresBox->ClearChildren();

	int32 i = 1;
	for (FPlayerScore PlayerScore : InPlayerScores)
	{
		UPlayerScoreWidget* PlayerScoreWidget = CreateWidget<UPlayerScoreWidget>(GetWorld(), PlayerScoreClass);
		if (!PlayerScoreWidget)
		{
			return;
		}
		PlayerScoreWidget->Setup(i, PlayerScore.PlayerName, PlayerScore.Score);
		PlayerScoresBox->AddChild(PlayerScoreWidget);

		// create a Spacer element and Add it to the PlayerScoresBox unless it is the last element
		if (i < InPlayerScores.Num())
		{
			USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
			Spacer->SetSize(FVector2d(1, 10));
			PlayerScoresBox->AddChild(Spacer);
		}
		
		++i;
	}
}