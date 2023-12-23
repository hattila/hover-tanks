// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HoverTankHUDWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHoverTankHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;
	
	UFUNCTION()
	void OnHealthChangeHandler(float InHealth, float InMaxHealth);

private:
	float Health;
	float MaxHealth;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* MaxHealthText;

	void RefreshHealth() const;
};
