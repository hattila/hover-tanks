// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamDeathMatchScoreBoardWidget.h"
#include "HoverTanks/Controllers/HoverTankPlayerController.h"
#include "PlayerScoreWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/Spacer.h"

UTeamDeathMatchScoreBoardWidget::UTeamDeathMatchScoreBoardWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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

bool UTeamDeathMatchScoreBoardWidget::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success)
	{
		return false;
	}

	if (!IsEveryElementInitialized())
	{
		return false;
	}
	
	JoinTeam1->OnClicked.AddDynamic(this, &UTeamDeathMatchScoreBoardWidget::AttemptToJoinTeam1);
	JoinTeam2->OnClicked.AddDynamic(this, &UTeamDeathMatchScoreBoardWidget::AttemptToJoinTeam2);

	return true;
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

	int32 Team1ScoreValue = 0;
	int32 Team2ScoreValue = 0;

	int32 i = 1;
	for (FPlayerScore PlayerScore : InPlayerScores)
	{
		UPlayerScoreWidget* PlayerScoreWidget = CreateWidget<UPlayerScoreWidget>(GetWorld(), PlayerScoreClass);
		if (!PlayerScoreWidget)
		{
			return;
		}
		PlayerScoreWidget->Setup(i, PlayerScore.PlayerName, PlayerScore.Score);

		if (PlayerScore.TeamId == INDEX_NONE)
		{
			// UE_LOG(LogTemp, Warning, TEXT("skipping player, with no team id"));
			continue;
		}

		if (PlayerScore.TeamId == 1)
		{
			PlayerScoresBoxTeam1->AddChild(PlayerScoreWidget);
			PlayerScoresBoxTeam1->AddChild(CreateSpacerElement());
			Team1ScoreValue += PlayerScore.Score;

			continue;
		}

		if (PlayerScore.TeamId == 2)
		{
			PlayerScoresBoxTeam2->AddChild(PlayerScoreWidget);
			PlayerScoresBoxTeam2->AddChild(CreateSpacerElement());
			Team2ScoreValue += PlayerScore.Score;
		}
	}

	Team1Score->SetText(FText::FromString(FString::FromInt(Team1ScoreValue)));
	Team2Score->SetText(FText::FromString(FString::FromInt(Team2ScoreValue)));
}

bool UTeamDeathMatchScoreBoardWidget::IsEveryElementInitialized() const
{
	if (PlayerScoresBoxTeam1 == nullptr)
	{
		return false;
	}

	if (PlayerScoresBoxTeam2 == nullptr)
	{
		return false;
	}
	
	if (JoinTeam1 == nullptr)
	{
		return false;
	}

	if (JoinTeam2 == nullptr)
	{
		return false;
	}
	
	return true;
}

USpacer* UTeamDeathMatchScoreBoardWidget::CreateSpacerElement() const
{
	USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
	Spacer->SetSize(FVector2d(1, 10));

	return Spacer;
}

void UTeamDeathMatchScoreBoardWidget::AttemptToJoinTeam(const uint8 TeamId) const
{
	if (!GetWorld())
	{
		return;
	}

	AHoverTankPlayerController* HoverTankPlayerController = GetOwningPlayer<AHoverTankPlayerController>();
	if (!HoverTankPlayerController)
	{
		return;	
	}

	HoverTankPlayerController->ServerAttemptToJoinTeam(TeamId);
}

void UTeamDeathMatchScoreBoardWidget::AttemptToJoinTeam1()
{
	AttemptToJoinTeam(1);
}

void UTeamDeathMatchScoreBoardWidget::AttemptToJoinTeam2()
{
	AttemptToJoinTeam(2);
}
