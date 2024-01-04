// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScoreBoardWidgetInterface.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "TeamDeathMatchScoreBoardWidget.generated.h"

class UScrollBox;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UTeamDeathMatchScoreBoardWidget : public UUserWidget, public IScoreBoardWidgetInterface
{
	GENERATED_BODY()

public:
	//~ Begin ScoreBoardWidgetInterface
	virtual void Setup() override;
	virtual void Teardown() override;

	virtual void SetMapName(const FString& InMapName) const override { MapName->SetText(FText::FromString(InMapName)); }
	virtual void SetGameModeName(const FString& InGameModeName) const override { GameModeName->SetText(FText::FromString(InGameModeName)); }
	virtual void SetTimeLeft(int32 InTimeLeft) override { TimeLeft = InTimeLeft; }
	virtual void RefreshTimeLeft() override;
	//~ End ScoreBoardWidgetInterface

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MapName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameModeName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeLeftText;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerScoresBoxTeam1;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerScoresBoxTeam2;
		
	TSubclassOf<UUserWidget> PlayerScoreClass;
	
	int32 TimeLeft = 0;
	FTimerHandle TimeLeftRefreshTimerHandle;
};
