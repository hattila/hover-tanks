// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DeathMatchHUD.generated.h"

class AHoverTankPlayerController;
class UHoverTankHUDWidget;
class ADeathMatchGameState;
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

	virtual void PostInitializeComponents() override;
	
	void ToggleScoreBoard();
	void ForceOpenScoreBoard();
	void RefreshPlayerScores();

	UFUNCTION()
	void OnPossessedPawnChangedHandler(APawn* OldPawn, APawn* NewPawn);
	
	UFUNCTION()
	void OnTankDeathHandler();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	ADeathMatchGameState* DeathMatchGameStateRef;

	TSubclassOf<UUserWidget> DeathMatchPlayerHUDWidgetClass; // timer, score, etc
	UDeathMatchPlayerHUDWidget* DeathMatchPlayerHUDWidget;

	TSubclassOf<UUserWidget> HoverTankHUDWidgetClass;
	UHoverTankHUDWidget* HoverTankHUDWidget;

	TSubclassOf<UUserWidget> DeathMatchScoreBoardClass; // TODO: change to ScoreBoardClass, which will have DM and TDM children
	UDeathMatchScoreBoardWidget* DeathMatchScoreBoardWidget;
};
