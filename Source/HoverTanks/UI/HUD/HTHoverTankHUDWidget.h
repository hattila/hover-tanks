// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HTWeaponCooldownWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "GameplayEffectTypes.h"

#include "HTHoverTankHUDWidget.generated.h"

class AHTHoverTank;
struct FOnAttributeChangeData;
class UBorder;
class UWidgetSwitcher;
class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTHoverTankHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// constructor
	UHTHoverTankHUDWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual bool Initialize() override;

	// UFUNCTION() // cannot be a UFUNCTION
	virtual void OnHealthAttributeChangeHandler(const FOnAttributeChangeData& Data);
	virtual void OnMaxHealthAttributeChangeHandler(const FOnAttributeChangeData& Data);
	
	virtual void OnShieldAttributeChangeHandler(const FOnAttributeChangeData& Data);
	virtual void OnMaxShieldAttributeChangeHandler(const FOnAttributeChangeData& Data);

	UFUNCTION()
	void OnWeaponFireHandler(int32 WeaponIndex, float CooldownTime);

	UFUNCTION()
	void OnWeaponSwitchedHandler(int32 NewWeapon);

	// create setters for the float attributes
	void SetShield(const float InShield) { Shield = InShield; }
	void SetMaxShield(const float InMaxShield) { MaxShield = InMaxShield; }
	void SetHealth(const float InHealth) { Health = InHealth; };
	void SetMaxHealth(const float InMaxHealth) { MaxHealth = InMaxHealth; };
	
	void RefreshProgressBars();

	// create a blueprint readable getter for the OwningHoverTank
	UFUNCTION(BlueprintCallable)
	AHTHoverTank* GetOwningHoverTank() const { return OwnerHoverTank; }
	
	void SetOwningHoverTank(AHTHoverTank* InHoverTank) { OwnerHoverTank = InHoverTank; OnHoverTankPawnSet(); }

	UFUNCTION(BlueprintImplementableEvent )
	void OnHoverTankPawnSet();
	
private:
	float Shield = 0;
	float MaxShield = 0;
	
	float Health = 0;
	float MaxHealth = 0;

	UPROPERTY(meta=(BindWidget))
	UProgressBar* ShieldProgressBar = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ShieldText = nullptr;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* MaxShieldText = nullptr;

	UPROPERTY(meta=(BindWidget))
	UProgressBar* HealthProgressBar	= nullptr;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* HealthText = nullptr;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* MaxHealthText = nullptr;

	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* WeaponIndicatorSwitch = nullptr;

	UPROPERTY(meta=(BindWidget))
	UBorder* CannonIndicator = nullptr;

	UPROPERTY(meta=(BindWidget))
	UBorder* RocketsIndicator = nullptr;

	TSubclassOf<UUserWidget> WeaponCooldownWidgetClass;

	UPROPERTY()
	UHTWeaponCooldownWidget* CannonCooldownWidget = nullptr;

	UPROPERTY()
	UHTWeaponCooldownWidget* RocketsCooldownWidget = nullptr;

	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* WeaponCooldownIndicatorsContainer = nullptr;

	UPROPERTY()
	AHTHoverTank* OwnerHoverTank = nullptr;
};
