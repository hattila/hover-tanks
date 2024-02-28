// Fill out your copyright notice in the Description page of Project Settings.


#include "HTToasterWidget.h"

bool UHTToasterWidget::Initialize()
{
	bool bIsSuccessful = Super::Initialize();
	if (!bIsSuccessful)
	{
		return false;
	}

	PlayAnimation(ShowAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.f);	
	
	FTimerHandle HideTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(HideTimerHandle, this, &UHTToasterWidget::HideDelayed, 5.f, false);

	return true;
}

void UHTToasterWidget::HideDelayed()
{
	if (ShowAnimation != nullptr)
	{
		PlayAnimation(ShowAnimation, 0.0f, 1, EUMGSequencePlayMode::Reverse, 1.f);	
	}

	FTimerHandle DestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &UHTToasterWidget::RemoveFromParent, .5f, false);
}
