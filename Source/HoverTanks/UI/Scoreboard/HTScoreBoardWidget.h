// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "UObject/Interface.h"
#include "HoverTanks/Game/HTPlayerScore.h"
#include "HoverTanks/MenuSystem/HTOpenableMenu.h"
#include "HTScoreBoardWidget.generated.h"

struct FHTPlayerScore;
class UScrollBox;
class UTextBlock;

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTScoreBoardWidget : public UHTOpenableMenu
{
	GENERATED_BODY()

public:
	virtual void Setup() override;
	virtual void Teardown() override;

	virtual void SetMapName(const FString& InMapName) const { MapName->SetText(FText::FromString(InMapName)); }
	virtual void SetGameModeName(const FString& InGameModeName) const { GameModeName->SetText(FText::FromString(InGameModeName)); }
	virtual void SetTimeLeft(int32 InTimeLeft) { TimeLeft = InTimeLeft; }
	
	virtual void RefreshTimeLeft();

	virtual void RefreshPlayerScores(const TArray<FHTPlayerScore>& InPlayerScores);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MapName = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameModeName = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeLeftText = nullptr;

	TSubclassOf<UUserWidget> PlayerScoreClass;
	
	int32 TimeLeft = 0;
	FTimerHandle TimeLeftRefreshTimerHandle;
};
