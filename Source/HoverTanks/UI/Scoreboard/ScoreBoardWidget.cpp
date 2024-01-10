// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreBoardWidget.h"

#include "PlayerScoreWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

void UScoreBoardWidget::Setup()
{
	AddToViewport();

	RefreshTimeLeft();
	GetWorld()->GetTimerManager().SetTimer(TimeLeftRefreshTimerHandle, this, &UScoreBoardWidget::RefreshTimeLeft, 1, true);
}

void UScoreBoardWidget::Teardown()
{
	GetWorld()->GetTimerManager().ClearTimer(TimeLeftRefreshTimerHandle);
	RemoveFromParent();
}

void UScoreBoardWidget::RefreshTimeLeft()
{
	// UE_LOG(LogTemp, Warning, TEXT("widget is calling a timed function"));
	
	if (TimeLeftText == nullptr)
	{
		return;
	}

	if (TimeLeft <= 0)
	{
		TimeLeftText->SetText(FText::FromString(TEXT("00:00")));
		return;
	}

	// format the time left
	int32 Minutes = TimeLeft / 60;
	int32 Seconds = TimeLeft % 60;
	FString TimeLeftString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);

	TimeLeftText->SetText(FText::FromString(TimeLeftString));

	TimeLeft--;
}

void UScoreBoardWidget::RefreshPlayerScores(const TArray<FPlayerScore>& InPlayerScores)
{
	return;
}
