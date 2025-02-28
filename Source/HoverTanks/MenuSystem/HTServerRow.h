﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UMG/Public/Blueprint/UserWidget.h"
#include "HTServerRow.generated.h"

class UHTMainMenu;
class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTServerRow : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(uint32 InIndex, UHTMainMenu* InMainMenu);

	void SetServerName(const FString& Name);
	void SetSessionIdString(const FString& InSessionIdString);

	void SetMapName(const FString& InMapName);
	void SetGameMode(const FString& InGameMode);
	
	void SetNumberOfPlayers(const FString& Players);
	void SetPing(const FString& InPing);
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ServerName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SessionIdString;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MapName;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameMode;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NumberOfPlayers;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Ping;
	
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	uint32 Index;

	UPROPERTY()
	UHTMainMenu* MainMenu;

	UFUNCTION()
	void OnJoinButtonClicked();
};
