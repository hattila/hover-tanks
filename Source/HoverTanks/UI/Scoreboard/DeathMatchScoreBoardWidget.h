// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScoreBoardWidget.h"
#include "HoverTanks/Game/PlayerScore.h"
#include "DeathMatchScoreBoardWidget.generated.h"

class UScrollBox;
class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UDeathMatchScoreBoardWidget : public UScoreBoardWidget
{
	GENERATED_BODY()
public:
	UDeathMatchScoreBoardWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual void RefreshPlayerScores(const TArray<FPlayerScore>& InPlayerScores) override;

private:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerScoresBox = nullptr;
	
};
