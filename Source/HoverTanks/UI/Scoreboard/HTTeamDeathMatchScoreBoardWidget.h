// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HTScoreBoardWidget.h"
#include "Components/TextBlock.h"
#include "HoverTanks/Game/HTPlayerScore.h"
#include "HTTeamDeathMatchScoreBoardWidget.generated.h"

class UButton;
class USpacer;
class UScrollBox;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTTeamDeathMatchScoreBoardWidget : public UHTScoreBoardWidget
{
	GENERATED_BODY()

public:
	UHTTeamDeathMatchScoreBoardWidget(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;
	
	virtual void RefreshPlayerScores(const TArray<FHTPlayerScore>& InPlayerScores) override;

protected:
	virtual bool IsEveryElementInitialized() const override;
	
private:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerScoresBoxTeam1 = nullptr;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerScoresBoxTeam2 = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team1Score = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team2Score = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinTeam1 = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinTeam2 = nullptr;
	
	USpacer* CreateSpacerElement() const;

	void AttemptToJoinTeam(const uint8 TeamId) const;
	
	UFUNCTION()
	void AttemptToJoinTeam1();

	UFUNCTION()
	void AttemptToJoinTeam2();
};
