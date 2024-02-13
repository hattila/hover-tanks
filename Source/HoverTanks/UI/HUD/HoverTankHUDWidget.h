// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponCooldownWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "GameplayEffectTypes.h"

#include "HoverTankHUDWidget.generated.h"

struct FOnAttributeChangeData;
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

	// UFUNCTION() // cannot be a UFUNCTION
	virtual void OnHealthAttributeChangeHandler(const FOnAttributeChangeData& Data);
	virtual void OnMaxHealthAttributeChangeHandler(const FOnAttributeChangeData& Data);
	
	virtual void OnShieldAttributeChangeHandler(const FOnAttributeChangeData& Data);
	virtual void OnMaxShieldAttributeChangeHandler(const FOnAttributeChangeData& Data);
	
	UFUNCTION()
	void OnHealthChangeHandler(float InHealth, float InMaxHealth);

	UFUNCTION()
	void OnWeaponFireHandler(int32 WeaponIndex, float CooldownTime);

	UFUNCTION()
	void OnWeaponSwitchedHandler(int32 NewWeapon);

private:
	float Shield;
	float MaxShield;
	
	float Health;
	float MaxHealth;

	UPROPERTY(meta=(BindWidget))
	UProgressBar* ShieldProgressBar;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* MaxShieldText;

	UPROPERTY(meta=(BindWidget))
	UProgressBar* HealthProgressBar;
	
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

	UPROPERTY()
	UWeaponCooldownWidget* CannonCooldownWidget = nullptr;

	UPROPERTY()
	UWeaponCooldownWidget* RocketsCooldownWidget = nullptr;

	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* WeaponCooldownIndicatorsContainer = nullptr;

	void RefreshHealth() const;
};
