// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DeathMatchHUD.generated.h"

class UDeathMatchScoreBoardWidget;
class UDeathMatchPlayerHUDWidget;
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

	void ToggleScoreBoard();
	void ForceOpenScoreBoard();
	void RefreshPlayerScores();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	TSubclassOf<UUserWidget> DeathMatchPlayerHUDWidgetClass; // timer, score, etc
	// TSubclassOf<UUserWidget> HoverTankHUDWidgetClass;
	
	UDeathMatchPlayerHUDWidget* DeathMatchPlayerHUDWidget;
	// UUserWidget* HoverTankHUDWidget;

	TSubclassOf<UUserWidget> DeathMatchScoreBoardClass; // TODO: change to ScoreBoardClass, which will have DM and TDM children
	UDeathMatchScoreBoardWidget* DeathMatchScoreBoardWidget;
};
