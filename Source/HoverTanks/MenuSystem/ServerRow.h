// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UMG/Public/Blueprint/UserWidget.h"
#include "ServerRow.generated.h"

class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UServerRow : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(uint32 InIndex);

	void SetServerName(const FString& Name);
	void SetNumberOfPlayers(const FString& Players);
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ServerName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NumberOfPlayers;
	
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	uint32 Index;

	UFUNCTION()
	void OnJoinButtonClicked();
};
