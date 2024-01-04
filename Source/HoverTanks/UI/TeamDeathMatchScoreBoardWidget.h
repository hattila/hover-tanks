// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScoreBoardWidget.h"
#include "Components/TextBlock.h"
#include "HoverTanks/Game/PlayerScore.h"
#include "TeamDeathMatchScoreBoardWidget.generated.h"

class UScrollBox;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UTeamDeathMatchScoreBoardWidget : public UScoreBoardWidget
{
	GENERATED_BODY()

public:
	UTeamDeathMatchScoreBoardWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual void RefreshPlayerScores(const TArray<FPlayerScore>& InPlayerScores) override;

private:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerScoresBoxTeam1 = nullptr;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerScoresBoxTeam2 = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team1Score = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team2Score = nullptr;
};
