// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "HTDeathMatchPlayerHUDWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTDeathMatchPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// constructor
	UHTDeathMatchPlayerHUDWidget(const FObjectInitializer& ObjectInitializer);
	
	void Setup();
	void Teardown();
	
	void SetTimeLeft(int32 InTimeLeft) { TimeLeft = InTimeLeft; }
	void RefreshTimeLeft();

	void ShowRespawnTextBorder(const bool bShow = true) const { RespawnTextBorder->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden); }

	void AddKillIndicator(
		const FString& KillerName,
		const FString& VictimName,
		FLinearColor KillerColor = FLinearColor(1,1,1,1),
		FLinearColor VictimColor = FLinearColor(1,1,1,1)
	);
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeLeftText = nullptr;

	int32 TimeLeft = 0;
	FTimerHandle TimeLeftRefreshTimerHandle;

	UPROPERTY(meta = (BindWidget))
	UBorder* RespawnTextBorder = nullptr;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* KillEventsVerticalBox = nullptr;

	TSubclassOf<UUserWidget> KillIndicatorWidgetClass;
};
