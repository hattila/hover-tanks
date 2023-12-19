// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "HoverTanks/Game/DeathMatchPlayerScore.h"
#include "DeathMatchScoreBoardWidget.generated.h"

class UScrollBox;
class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UDeathMatchScoreBoardWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UDeathMatchScoreBoardWidget(const FObjectInitializer& ObjectInitializer);

	// Initialize
	virtual bool Initialize() override;
	
	void Setup();
	void Teardown();

	void SetMapName(const FString& InMapName) const { MapName->SetText(FText::FromString(InMapName)); }
	void SetGameModeName(const FString& InGameModeName) const { GameModeName->SetText(FText::FromString(InGameModeName)); }
	void SetTimeLeft(int32 InTimeLeft) { TimeLeft = InTimeLeft; }

	void RefreshPlayerScores(const TArray<FDeathMatchPlayerScore>& InPlayerScores);
	void RefreshTimeLeft();

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
	
	int32 TimeLeft;
	FTimerHandle TimeLeftRefreshTimerHandle;
};
