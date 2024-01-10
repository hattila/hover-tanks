// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerScoreWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UPlayerScoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(int32 InPlacement, const FString& InPlayerName, int32 InScore);
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Placement;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Score;
};
