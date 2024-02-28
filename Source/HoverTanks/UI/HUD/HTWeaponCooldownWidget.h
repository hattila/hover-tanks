// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "HTWeaponCooldownWidget.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTWeaponCooldownWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// constructor
	UHTWeaponCooldownWidget(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;
	
	void StartCooldownTimer(float InCooldownTime);
	void SetProgressBarPercent(const float InPercent) const { ProgressBar->SetPercent(InPercent); }

private:
	UPROPERTY(meta=(BindWidget))
	UProgressBar* ProgressBar;

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	UWidgetAnimation* CooldownAnimation;

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	UWidgetAnimation* ShowAnimation;

	float CooldownTime = 0.0f;

	void HideDelayed();
};
