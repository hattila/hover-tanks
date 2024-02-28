// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HTKillIndicatorWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTKillIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(
		const FString& KillerName,
		const FString& VictimName,
		FLinearColor KillerColor = FLinearColor(1,1,1,1),
		FLinearColor VictimColor = FLinearColor(1,1,1,1)
	);
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillerNameText = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* VictimNameText = nullptr;

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	UWidgetAnimation* ShowAnimation;

	void HideDelayed();
	
};
