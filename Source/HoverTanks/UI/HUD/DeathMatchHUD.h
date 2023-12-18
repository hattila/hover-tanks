// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DeathMatchHUD.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API ADeathMatchHUD : public AHUD
{
	GENERATED_BODY()

public:
	// create a constructor
	ADeathMatchHUD();

protected:
	virtual void BeginPlay() override;
	
private:

	TSubclassOf<UUserWidget> DeathMatchPlayerHUDWidgetClass;
	// TSubclassOf<UUserWidget> HoverTankHUDWidgetClass;
	
	UUserWidget* DeathMatchPlayerHUDWidget;
	// UUserWidget* HoverTankHUDWidget;
};
