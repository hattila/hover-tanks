// Fill out your copyright notice in the Description page of Project Settings.


#include "HTWeaponCooldownWidget.h"

UHTWeaponCooldownWidget::UHTWeaponCooldownWidget(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer),
	ProgressBar(nullptr),
	CooldownAnimation(nullptr),
	ShowAnimation(nullptr)
{
	
}

bool UHTWeaponCooldownWidget::Initialize()
{
	const bool bIsSuccessfullyInitialized = Super::Initialize();

	if (!bIsSuccessfullyInitialized)
	{
		return false;
	}

	SetRenderOpacity(0);
	
	return true;
}

void UHTWeaponCooldownWidget::StartCooldownTimer(const float InCooldownTime)
{
	ProgressBar->SetPercent(0);
	
	if (CooldownAnimation != nullptr && ShowAnimation != nullptr)
	{
		// UE_LOG(LogTemp, Warning, TEXT("UHTWeaponCooldownWidget CooldownAnimation found"));
		
		float PlaybackSpeed = 1.0f / InCooldownTime;
		PlayAnimation(ShowAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 5.f);
		PlayAnimation(CooldownAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, PlaybackSpeed);

		FTimerHandle HideTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(HideTimerHandle, this, &UHTWeaponCooldownWidget::HideDelayed, InCooldownTime - 0.1f, false);
	}
	
}

void UHTWeaponCooldownWidget::HideDelayed()
{
	if (ShowAnimation != nullptr)
	{
		PlayAnimation(ShowAnimation, 0.0f, 1, EUMGSequencePlayMode::Reverse, 5.f);	
	}
}
