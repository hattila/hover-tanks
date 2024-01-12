// Fill out your copyright notice in the Description page of Project Settings.


#include "KillIndicatorWidget.h"

#include "Components/TextBlock.h"

void UKillIndicatorWidget::Setup(const FString& KillerName, const FString& VictimName, FLinearColor KillerColor, FLinearColor VictimColor)
{
	if (ShowAnimation != nullptr)
	{
		PlayAnimation(ShowAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}
	
	KillerColor.A = 0.9f;
	VictimColor.A = 0.9f;
	
	KillerNameText->SetText(FText::FromString(KillerName));
	KillerNameText->SetColorAndOpacity(KillerColor);

	VictimNameText->SetText(FText::FromString(VictimName));
	VictimNameText->SetColorAndOpacity(VictimColor);

	// create a timer to remove the widget after a few seconds
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UKillIndicatorWidget::HideDelayed, 5.0f, false);
}

void UKillIndicatorWidget::HideDelayed()
{
	if (ShowAnimation != nullptr)
	{
		PlayAnimation(ShowAnimation, 0.0f, 1, EUMGSequencePlayMode::Reverse, 1.f);	
	}

	FTimerHandle DestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &UKillIndicatorWidget::RemoveFromParent, .5f, false);
}
