// Fill out your copyright notice in the Description page of Project Settings.


#include "HTDeathMatchScoreBoardWidget.h"

#include "Components/ScrollBox.h"
#include "HTPlayerScoreWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Spacer.h"

UHTDeathMatchScoreBoardWidget::UHTDeathMatchScoreBoardWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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

void UHTDeathMatchScoreBoardWidget::RefreshPlayerScores(const TArray<FHTPlayerScore>& InPlayerScores)
{
	if (PlayerScoreClass == nullptr || PlayerScoresBox == nullptr)
	{
		return;
	}

	PlayerScoresBox->ClearChildren();

	int32 i = 1;
	for (FHTPlayerScore PlayerScore : InPlayerScores)
	{
		UHTPlayerScoreWidget* PlayerScoreWidget = CreateWidget<UHTPlayerScoreWidget>(GetWorld(), PlayerScoreClass);
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