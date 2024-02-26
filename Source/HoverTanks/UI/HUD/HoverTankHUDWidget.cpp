// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankHUDWidget.h"

#include "AbilitySystemComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

UHoverTankHUDWidget::UHoverTankHUDWidget(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
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

	// set their widths to 15
	CannonCooldownWidget->SetDesiredSizeInViewport(FVector2d(15, 75)); // does nothing currently

	WeaponCooldownIndicatorsContainer->AddChild(CannonCooldownWidget);
	// add a spacer
	USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
	Spacer->SetSize(FVector2d(5, 5));
	WeaponCooldownIndicatorsContainer->AddChild(Spacer);
	WeaponCooldownIndicatorsContainer->AddChild(RocketsCooldownWidget);
	
	return true;
}

void UHoverTankHUDWidget::OnHealthAttributeChangeHandler(const FOnAttributeChangeData& Data)
{
	Health = Data.NewValue;
	HealthText->SetText(FText::AsNumber(Health));
	HealthProgressBar->SetPercent(Health / MaxHealth);
}

void UHoverTankHUDWidget::OnMaxHealthAttributeChangeHandler(const FOnAttributeChangeData& Data)
{
	MaxHealth = Data.NewValue;
	MaxHealthText->SetText(FText::AsNumber(MaxHealth));
	HealthProgressBar->SetPercent(Health / MaxHealth);
}

void UHoverTankHUDWidget::OnShieldAttributeChangeHandler(const FOnAttributeChangeData& Data)
{
	Shield = Data.NewValue;
	ShieldText->SetText(FText::AsNumber(Shield));
	ShieldProgressBar->SetPercent(Shield / MaxShield);
}

void UHoverTankHUDWidget::OnMaxShieldAttributeChangeHandler(const FOnAttributeChangeData& Data)
{
	MaxShield = Data.NewValue;
	MaxShieldText->SetText(FText::AsNumber(MaxShield));
	ShieldProgressBar->SetPercent(Shield / MaxShield);
}

void UHoverTankHUDWidget::RefreshProgressBars()
{
	HealthProgressBar->SetPercent(Health / MaxHealth);
	ShieldProgressBar->SetPercent(Shield / MaxShield);
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
		break;
		// case EAvailableWeapons::RocketLauncher:
		case 1:
			WeaponIndicatorSwitch->SetActiveWidget(RocketsIndicator);
		break;
	default:
		return;
	}
}
