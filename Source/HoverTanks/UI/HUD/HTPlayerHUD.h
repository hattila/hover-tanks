// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HT_ScoringHUDInterface.h"
#include "GameFramework/HUD.h"
#include "HTPlayerHUD.generated.h"

class AHTHoverTank;
class UAbilitySystemComponent;
class UScoreBoardWidget;
class UHTHoverTankHUDWidget;
class UHTDeathMatchPlayerHUDWidget;

/**
 * 
 */
UCLASS()
class HOVERTANKS_API AHTPlayerHUD : public AHUD, public IHT_ScoringHUDInterface
{
	GENERATED_BODY()

public:
	AHTPlayerHUD();

	virtual void PostInitializeComponents() override;
	
	void CreatePlayerHUD();
	void CreateTankHUD(AHTHoverTank* HoverTank);
	
	// ~IScoringHUDInterface
	virtual void ToggleScoreBoard() override;
	virtual void ForceOpenScoreBoard() override;
	virtual void RefreshPlayerScores() override;

	virtual void AddKillIndicator(
		const FString& KillerName,
		const FString& VictimName,
		FLinearColor KillerColor = FLinearColor(1,1,1,1),
		FLinearColor VictimColor = FLinearColor(1,1,1,1)
	) override;
	// ~IScoringHUDInterface
	
	UFUNCTION()
	void OnTankDeathHandler();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	TSubclassOf<UUserWidget> PlayerHUDWidgetClass; // timer, score, etc

	UPROPERTY()
	UHTDeathMatchPlayerHUDWidget* PlayerHUDWidget = nullptr;
	
	TSubclassOf<UUserWidget> HoverTankHUDWidgetClass;

	UPROPERTY()
	UHTHoverTankHUDWidget* HoverTankHUDWidget = nullptr;

	TSubclassOf<UUserWidget> ScoreBoardClass;

	UPROPERTY()
	UScoreBoardWidget* ScoreBoardWidget = nullptr;

private:
	AGameStateBase* GetSafeGameState() const;

	void SetupAbilitySystemAttributeChangeHandlers(UAbilitySystemComponent* AbilitySystemComponent);
	
};
