// Fill out your copyright notice in the Description page of Project Settings.


#include "ToasterWidget.h"

bool UToasterWidget::Initialize()
{
	bool bIsSuccessful = Super::Initialize();
	if (!bIsSuccessful)
	{
		return false;
	}

	PlayAnimation(ShowAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.f);	
	
	FTimerHandle HideTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(HideTimerHandle, this, &UToasterWidget::HideDelayed, 5.f, false);

	return true;
}

void UToasterWidget::HideDelayed()
{
	if (ShowAnimation != nullptr)
	{
		PlayAnimation(ShowAnimation, 0.0f, 1, EUMGSequencePlayMode::Reverse, 1.f);	
	}

	FTimerHandle DestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &UToasterWidget::RemoveFromParent, .5f, false);
}
