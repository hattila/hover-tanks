// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScoringHUDInterface.h"
#include "GameFramework/HUD.h"
#include "DeathMatchHUD.generated.h"

class AGameState;
class UScoreBoardWidget;
class AHoverTankPlayerController;
class UHoverTankHUDWidget;
class ADeathMatchGameState;
class UDeathMatchPlayerHUDWidget;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API ADeathMatchHUD : public AHUD, public IScoringHUDInterface
{
	GENERATED_BODY()

public:
	// create a constructor
	ADeathMatchHUD();

	virtual void PostInitializeComponents() override;

	// ~IScoringHUDInterface
	virtual void ToggleScoreBoard() override;
	virtual void ForceOpenScoreBoard() override;
	virtual void RefreshPlayerScores() override;
	// ~IScoringHUDInterface
	
	UFUNCTION()
	void OnPossessedPawnChangedHandler(APawn* OldPawn, APawn* NewPawn);
	
	UFUNCTION()
	void OnTankDeathHandler();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	TSubclassOf<UUserWidget> PlayerHUDWidgetClass; // timer, score, etc

	UPROPERTY()
	UDeathMatchPlayerHUDWidget* PlayerHUDWidget = nullptr;
	
	TSubclassOf<UUserWidget> HoverTankHUDWidgetClass;

	UPROPERTY()
	UHoverTankHUDWidget* HoverTankHUDWidget = nullptr;

	TSubclassOf<UUserWidget> ScoreBoardClass;

	UPROPERTY()
	UScoreBoardWidget* ScoreBoardWidget = nullptr;

private:
	AGameStateBase* GetSafeGameState() const;
};
