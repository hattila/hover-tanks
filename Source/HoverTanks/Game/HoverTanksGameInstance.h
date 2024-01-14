// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiplayerGameControls.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "UObject/Object.h"
#include "HoverTanksGameInstance.generated.h"

class UMainMenu;

const static FName GHover_Tanks_Session_Name = TEXT("My Hover Tanks Game Session");

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHoverTanksGameInstance : public UGameInstance, public IMultiplayerGameControls
{
	GENERATED_BODY()

public:
	UHoverTanksGameInstance(const FObjectInitializer& ObjectInitializer);
	
	virtual void Init();

	UFUNCTION(BlueprintCallable)
	void ShowMainMenu();

	// ~IMultiplayerGameControls
	UFUNCTION(Exec)
	void Host();
	UFUNCTION(Exec)
	void Join(const FString& Address);

	void HostGame(const FHostGameSettings& InHostGameSettings);
	void RefreshServerList();
	void JoinAvailableGame(uint32 Index);
	// ~IMultiplayerGameControls

	UFUNCTION(Exec)
	void InputMode(const FString& InInputMode);

	void AddToastMessage(const FString& String, const bool bShowLoading = false);

private:
	/**
	 * Menu System
	 */

	TSubclassOf<UUserWidget> MainMenuClass;

	TSubclassOf<UUserWidget> ToasterWidgetClass;

	UMainMenu* MainMenu;
	
	/**
	 * Online Session
	 */
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FHostGameSettings HostGameSettings;
	
	void StartCreateSession();
	
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void OnSessionUserInviteAccepted(bool bWasSuccess, int ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);

};
