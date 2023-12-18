// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchPlayerHUDWidget.h"

#include "Components/TextBlock.h"

void UDeathMatchPlayerHUDWidget::Setup()
{
	AddToViewport();

	RefreshTimeLeft();
	GetWorld()->GetTimerManager().SetTimer(TimeLeftRefreshTimerHandle, this, &UDeathMatchPlayerHUDWidget::RefreshTimeLeft, 1, true);
}

void UDeathMatchPlayerHUDWidget::Teardown()
{
	GetWorld()->GetTimerManager().ClearTimer(TimeLeftRefreshTimerHandle);

	RemoveFromParent();
}

/**
 * Exact copy of DeathMatchScoreBoardWidget's timer refresh function
 */
void UDeathMatchPlayerHUDWidget::RefreshTimeLeft()
{
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
