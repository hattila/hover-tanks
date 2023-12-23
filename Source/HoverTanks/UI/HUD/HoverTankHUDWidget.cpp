// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankHUDWidget.h"

#include "Components/TextBlock.h"

bool UHoverTankHUDWidget::Initialize()
{
	return Super::Initialize();
}

void UHoverTankHUDWidget::RefreshHealth() const
{
	HealthText->SetText(FText::AsNumber(Health));
	MaxHealthText->SetText(FText::AsNumber(MaxHealth));
}

void UHoverTankHUDWidget::OnHealthChangeHandler(const float InHealth, const float InMaxHealth)
{
	Health = InHealth;
	MaxHealth = InMaxHealth;

	RefreshHealth();
}
