// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "UObject/Interface.h"
#include "HoverTanks/Game/PlayerScore.h"
#include "ScoreBoardWidget.generated.h"

struct FPlayerScore;
class UScrollBox;
class UTextBlock;

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UScoreBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Initialize
	virtual bool Initialize() override;
	
	virtual void Setup();
	virtual void Teardown();

	virtual void SetMapName(const FString& InMapName) const { MapName->SetText(FText::FromString(InMapName)); }
	virtual void SetGameModeName(const FString& InGameModeName) const { GameModeName->SetText(FText::FromString(InGameModeName)); }
	virtual void SetTimeLeft(int32 InTimeLeft) { TimeLeft = InTimeLeft; }
	
	virtual void RefreshTimeLeft();

	virtual void RefreshPlayerScores(const TArray<FPlayerScore>& InPlayerScores);

	/**
	 * Could this be an OpenableMenu?
	 */
	void SetupInputModeGameAndUi();
	void SetInputModeGameOnly() const;

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
