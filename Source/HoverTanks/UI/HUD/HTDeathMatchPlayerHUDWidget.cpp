// Fill out your copyright notice in the Description page of Project Settings.


#include "HTDeathMatchPlayerHUDWidget.h"

#include "HTKillIndicatorWidget.h"
#include "Components/TextBlock.h"

UHTDeathMatchPlayerHUDWidget::UHTDeathMatchPlayerHUDWidget(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> KillIndicatorWidgetClassFinder(
		TEXT("/Game/HoverTanks/UI/HUD/WBP_KillIndicatorWidget"));
	KillIndicatorWidgetClass = KillIndicatorWidgetClassFinder.Class;
}

void UHTDeathMatchPlayerHUDWidget::Setup()
{
	AddToViewport();

	RefreshTimeLeft();
	GetWorld()->GetTimerManager().SetTimer(TimeLeftRefreshTimerHandle, this, &UHTDeathMatchPlayerHUDWidget::RefreshTimeLeft, 1, true);
}

void UHTDeathMatchPlayerHUDWidget::Teardown()
{
	GetWorld()->GetTimerManager().ClearTimer(TimeLeftRefreshTimerHandle);

	RemoveFromParent();
}

/**
 * Exact copy of ScoreBoardWidget's timer refresh function
 */
void UHTDeathMatchPlayerHUDWidget::RefreshTimeLeft()
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

void UHTDeathMatchPlayerHUDWidget::AddKillIndicator(const FString& KillerName, const FString& VictimName, FLinearColor KillerColor, FLinearColor VictimColor)
{
	UHTKillIndicatorWidget* KillIndicatorWidget = CreateWidget<UHTKillIndicatorWidget>(GetOwningPlayer(), KillIndicatorWidgetClass);
	if (KillIndicatorWidget == nullptr)
	{
		return;
	}

	KillIndicatorWidget->Setup(KillerName, VictimName, KillerColor, VictimColor);
	KillEventsVerticalBox->AddChild(KillIndicatorWidget);
}
