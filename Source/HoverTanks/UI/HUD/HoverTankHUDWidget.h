// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DeathMatchHUD.h"
#include "WeaponCooldownWidget.h"
#include "Blueprint/UserWidget.h"
#include "HoverTanks/Components/WeaponsComponent.h"
#include "HoverTankHUDWidget.generated.h"

class UBorder;
class UWidgetSwitcher;
class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHoverTankHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// constructor
	UHoverTankHUDWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual bool Initialize() override;
	
	UFUNCTION()
	void OnHealthChangeHandler(float InHealth, float InMaxHealth);

	UFUNCTION()
	void OnWeaponFireHandler(int32 WeaponIndex, float CooldownTime);

	UFUNCTION()
	void OnWeaponSwitchedHandler(int32 NewWeapon);

private:
	float Health;
	float MaxHealth;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* MaxHealthText;

	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* WeaponIndicatorSwitch;

	UPROPERTY(meta=(BindWidget))
	UBorder* CannonIndicator;

	UPROPERTY(meta=(BindWidget))
	UBorder* RocketsIndicator;

	TSubclassOf<UUserWidget> WeaponCooldownWidgetClass;
	UWeaponCooldownWidget* CannonCooldownWidget = nullptr;
	UWeaponCooldownWidget* RocketsCooldownWidget = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* WeaponCooldownIndicatorSwitch;

	void RefreshHealth() const;
};
