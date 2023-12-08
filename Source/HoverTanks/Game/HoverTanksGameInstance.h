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
	UHoverTanksGameInstance(const FObjectInitializer& ObjectInitializer);
	
	virtual void Init();

	UFUNCTION(BlueprintCallable)
	void ShowMainMenu();
	
	UFUNCTION(Exec)
	void Host();

	UFUNCTION(Exec)
	void Join(const FString& Address);

private:
	/**
	 * Menu System
	 */

	TSubclassOf<UUserWidget> MainMenuClass;
	
	/**
	 * Online Session
	 */

	IOnlineSessionPtr SessionInterface;

	void StartCreateSession();
	
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void OnSessionUserInviteAccepted(bool bWasSuccess, int ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);

};
