// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchPlayerHUDWidget.h"

#include "KillIndicatorWidget.h"
#include "Components/TextBlock.h"

UDeathMatchPlayerHUDWidget::UDeathMatchPlayerHUDWidget(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> KillIndicatorWidgetClassFinder(
		TEXT("/Game/HoverTanks/UI/HUD/WBP_KillIndicatorWidget"));
	KillIndicatorWidgetClass = KillIndicatorWidgetClassFinder.Class;
}

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
 * Exact copy of ScoreBoardWidget's timer refresh function
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

void UDeathMatchPlayerHUDWidget::AddKillIndicator(const FString& KillerName, const FString& VictimName, FLinearColor KillerColor, FLinearColor VictimColor)
{
	UKillIndicatorWidget* KillIndicatorWidget = CreateWidget<UKillIndicatorWidget>(GetOwningPlayer(), KillIndicatorWidgetClass);
	if (KillIndicatorWidget == nullptr)
	{
		return;
	}

	KillIndicatorWidget->Setup(KillerName, VictimName, KillerColor, VictimColor);
	KillEventsVerticalBox->AddChild(KillIndicatorWidget);
}
