// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "HoverTanks/Game/DeathMatchPlayerScore.h"
#include "DeathMatchScoreBoard.generated.h"

class UScrollBox;
class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UDeathMatchScoreBoard : public UUserWidget
{
	GENERATED_BODY()
public:
	UDeathMatchScoreBoard(const FObjectInitializer& ObjectInitializer);

	void Setup();
	void Teardown();

	void SetMapName(const FString& InMapName) const { MapName->SetText(FText::FromString(InMapName)); }
	void SetGameModeName(const FString& InGameModeName) const { GameModeName->SetText(FText::FromString(InGameModeName)); }
	void SetTimeLeft(float InTimeLeft) { TimeLeft = InTimeLeft; }

	void RefreshPlayerScores(const TArray<FDeathMatchPlayerScore> InPlayerScores);
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MapName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameModeName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeLeftText;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerScoresBox;

	TSubclassOf<UUserWidget> PlayerScoreClass;

	float TimeLeft;
	TArray<FDeathMatchPlayerScore> PlayerScoresArray;

	void ReDrawPlayerScores();
};
