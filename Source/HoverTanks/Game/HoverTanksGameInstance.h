// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "UObject/Object.h"
#include "HoverTanksGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHoverTanksGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// constructor
	UHoverTanksGameInstance(const FObjectInitializer& ObjectInitializer);
	
	// init function
	virtual void Init();

	UFUNCTION(Exec)
	void Host();

	UFUNCTION(Exec)
	void Join(const FString& Address);

private:
	IOnlineSessionPtr SessionInterface;

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

};
