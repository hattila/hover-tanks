// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerScoreWidget.h"

#include "Components/TextBlock.h"

void UPlayerScoreWidget::Setup(int32 InPlacement, const FString& InPlayerName, int32 InScore)
{
	Placement->SetText(FText::FromString(FString::FromInt(InPlacement)));
	PlayerName->SetText(FText::FromString(InPlayerName));
	Score->SetText(FText::FromString(FString::FromInt(InScore)));	
}