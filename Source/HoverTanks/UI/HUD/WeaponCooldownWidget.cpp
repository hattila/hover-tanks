// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponCooldownWidget.h"

void UWeaponCooldownWidget::SetProgressBarPercent(const float InPercent) const
{
	ProgressBar->SetPercent(InPercent);
}
