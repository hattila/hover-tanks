// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HTScoreBoardWidget.h"
#include "HoverTanks/Game/HTPlayerScore.h"
#include "HTDeathMatchScoreBoardWidget.generated.h"

class UScrollBox;
class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTDeathMatchScoreBoardWidget : public UHTScoreBoardWidget
{
	GENERATED_BODY()
public:
	UHTDeathMatchScoreBoardWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual void RefreshPlayerScores(const TArray<FHTPlayerScore>& InPlayerScores) override;

private:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerScoresBox = nullptr;
	
};
