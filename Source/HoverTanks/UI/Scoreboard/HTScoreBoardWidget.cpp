// Fill out your copyright notice in the Description page of Project Settings.


#include "HTScoreBoardWidget.h"

#include "HTPlayerScoreWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

void UHTScoreBoardWidget::Setup()
{
	AddToViewport();

	RefreshTimeLeft();
	GetWorld()->GetTimerManager().SetTimer(TimeLeftRefreshTimerHandle, this, &UHTScoreBoardWidget::RefreshTimeLeft, 1, true);
}

void UHTScoreBoardWidget::Teardown()
{
	GetWorld()->GetTimerManager().ClearTimer(TimeLeftRefreshTimerHandle);
	RemoveFromParent();
}

void UHTScoreBoardWidget::RefreshTimeLeft()
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

void UHTScoreBoardWidget::RefreshPlayerScores(const TArray<FHTPlayerScore>& InPlayerScores)
{
	return;
}
