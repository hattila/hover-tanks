// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamDeathMatchScoreBoardWidget.h"

#include "DeathMatchPlayerScoreWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/Spacer.h"

UTeamDeathMatchScoreBoardWidget::UTeamDeathMatchScoreBoardWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// initialize the player score class
	static ConstructorHelpers::FClassFinder<UUserWidget> PlayerScoreClassFinder(
		TEXT("/Game/HoverTanks/UI/WBP_DeathMatchPlayerScoreWidget"));
	if (!ensure(PlayerScoreClassFinder.Class != nullptr))
	{
		return;
	}
	PlayerScoreClass = PlayerScoreClassFinder.Class;
}

void UTeamDeathMatchScoreBoardWidget::RefreshPlayerScores(const TArray<FPlayerScore>& InPlayerScores)
{
	if (PlayerScoreClass == nullptr || PlayerScoresBoxTeam1 == nullptr || PlayerScoresBoxTeam2 == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTeamDeathMatchScoreBoardWidget::RefreshPlayerScores, sum ting wong"));
		return;
	}

	PlayerScoresBoxTeam1->ClearChildren();
	PlayerScoresBoxTeam2->ClearChildren();

	int32 i = 1;
	for (FPlayerScore PlayerScore : InPlayerScores)
	{
		UDeathMatchPlayerScoreWidget* PlayerScoreWidget = CreateWidget<UDeathMatchPlayerScoreWidget>(GetWorld(), PlayerScoreClass);
		if (!PlayerScoreWidget)
		{
			return;
		}
		PlayerScoreWidget->Setup(i, PlayerScore.PlayerName, PlayerScore.Score);
		PlayerScoresBoxTeam1->AddChild(PlayerScoreWidget);
		PlayerScoresBoxTeam2->AddChild(PlayerScoreWidget);

		// create a Spacer element and Add it to the PlayerScoresBox unless it is the last element
		if (i < InPlayerScores.Num())
		{
			USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
			Spacer->SetSize(FVector2d(1, 10));
			PlayerScoresBoxTeam1->AddChild(Spacer);
			PlayerScoresBoxTeam2->AddChild(Spacer);
		}
		
		++i;
	}
}