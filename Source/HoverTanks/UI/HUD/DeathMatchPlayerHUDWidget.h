// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathMatchPlayerHUDWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UDeathMatchPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void Setup();
	void Teardown();
	
	void SetTimeLeft(int32 InTimeLeft) { TimeLeft = InTimeLeft; }
	void RefreshTimeLeft();
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeLeftText;

	int32 TimeLeft;
	FTimerHandle TimeLeftRefreshTimerHandle;
};
