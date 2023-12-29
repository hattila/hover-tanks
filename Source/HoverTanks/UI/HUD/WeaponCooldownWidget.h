// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "WeaponCooldownWidget.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UWeaponCooldownWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetProgressBarPercent(float InPercent) const;
	
private:
	UPROPERTY(meta=(BindWidget))
	UProgressBar* ProgressBar;
};
