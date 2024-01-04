// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamDeathMatchScoreBoardWidget.h"

void UTeamDeathMatchScoreBoardWidget::Setup()
{
	AddToViewport();

	RefreshTimeLeft();
	GetWorld()->GetTimerManager().SetTimer(TimeLeftRefreshTimerHandle, this, &UTeamDeathMatchScoreBoardWidget::RefreshTimeLeft, 1, true);
}

void UTeamDeathMatchScoreBoardWidget::Teardown()
{
	GetWorld()->GetTimerManager().ClearTimer(TimeLeftRefreshTimerHandle);
	
	RemoveFromParent();
}

void UTeamDeathMatchScoreBoardWidget::RefreshTimeLeft()
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