// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankHUDWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

UHoverTankHUDWidget::UHoverTankHUDWidget(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer),
	Health(0),
	MaxHealth(0),
	HealthText(nullptr),
	MaxHealthText(nullptr),
	WeaponIndicatorSwitch(nullptr),
	CannonIndicator(nullptr),
	RocketsIndicator(nullptr),
	WeaponCooldownIndicatorSwitch(nullptr)
{
	// get the blueprint version of the weapon cooldown widget class
	static ConstructorHelpers::FClassFinder<UUserWidget> WeaponCooldownWidgetClassFinder(
		TEXT("/Game/HoverTanks/UI/HUD/WBP_WeaponCooldownWidget"));
	if (WeaponCooldownWidgetClassFinder.Succeeded())
	{
		WeaponCooldownWidgetClass = WeaponCooldownWidgetClassFinder.Class;
	}
}

bool UHoverTankHUDWidget::Initialize()
{
	const bool bIsSuccessfullyInitialized = Super::Initialize();

	if (!bIsSuccessfullyInitialized)
	{
		return false;
	}

	CannonCooldownWidget = CreateWidget<UWeaponCooldownWidget>(GetWorld(), WeaponCooldownWidgetClass);
	if (!ensure(CannonCooldownWidget != nullptr))
	{
		return false;
	}

	RocketsCooldownWidget = CreateWidget<UWeaponCooldownWidget>(GetWorld(), WeaponCooldownWidgetClass);
	if (!ensure(RocketsCooldownWidget != nullptr))
	{
		return false;
	}

	CannonCooldownWidget->SetProgressBarPercent(0.2f);
	RocketsCooldownWidget->SetProgressBarPercent(0.6f);

	// add them to the switcher
	WeaponCooldownIndicatorSwitch->AddChild(CannonCooldownWidget);
	WeaponCooldownIndicatorSwitch->AddChild(RocketsCooldownWidget);

	WeaponCooldownIndicatorSwitch->SetActiveWidget(CannonCooldownWidget);
	
	return true;
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

void UHoverTankHUDWidget::OnWeaponFireHandler(const int32 WeaponIndex, const float CooldownTime)
{
	switch (WeaponIndex)
	{
		// case EAvailableWeapons::Cannon:
		case 0:
			CannonCooldownWidget->StartCooldownTimer(CooldownTime);
		break;
		// case EAvailableWeapons::RocketLauncher:
		case 1:
			RocketsCooldownWidget->StartCooldownTimer(CooldownTime);
		break;
	default:
		return;
	}
}

void UHoverTankHUDWidget::OnWeaponSwitchedHandler(int32 NewWeapon)
{
	// UE_LOG(LogTemp, Warning, TEXT("UHoverTankHUDWidget::OnWeaponSwitchedHandler, NewWeapon is %d"), NewWeapon);
	
	switch (NewWeapon)
	{
		// case EAvailableWeapons::Cannon:
		case 0:
			WeaponIndicatorSwitch->SetActiveWidget(CannonIndicator);
			WeaponCooldownIndicatorSwitch->SetActiveWidget(CannonCooldownWidget);
			// CannonCooldownWidget->StartCooldownTimer(1.0f);
		break;
		// case EAvailableWeapons::RocketLauncher:
		case 1:
			WeaponIndicatorSwitch->SetActiveWidget(RocketsIndicator);
			WeaponCooldownIndicatorSwitch->SetActiveWidget(RocketsCooldownWidget);
			// RocketsCooldownWidget->StartCooldownTimer(3.0f);
		break;
	default:
		return;
	}
}
