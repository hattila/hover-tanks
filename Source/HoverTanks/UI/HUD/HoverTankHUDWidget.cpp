// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankHUDWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "HoverTanks/Components/WeaponsComponent.h"

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

void UHoverTankHUDWidget::OnWeaponSwitchedHandler(int32 NewWeapon)
{
	// UE_LOG(LogTemp, Warning, TEXT("UHoverTankHUDWidget::OnWeaponSwitchedHandler, NewWeapon is %d"), NewWeapon);
	
	switch (NewWeapon)
	{
		// case EAvailableWeapons::Cannon:
		case 0:
			WeaponIndicatorSwitch->SetActiveWidget(CannonIndicator);
		break;
		// case EAvailableWeapons::RocketLauncher:
		case 1:
			WeaponIndicatorSwitch->SetActiveWidget(RocketsIndicator);
		break;
	default:
		return;
	}
}
