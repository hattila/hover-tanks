// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OpenableMenu.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"


class IMultiplayerGameControls;
struct FAvailableGame;

class UThrobber;
class UServerRow;
class UHoverTanksGameInstance;
class UWidgetSwitcher;
class UButton;

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UMainMenu : public UOpenableMenu
{
	GENERATED_BODY()

public:
	explicit UMainMenu(const FObjectInitializer& ObjectInitializer);
	void SetMultiplayerGameControls(IMultiplayerGameControls* InMultiplayerGameControls) { MultiplayerGameControls = InMultiplayerGameControls; }

	virtual bool Initialize() override;
	virtual void Setup() override;
	virtual void Teardown() override;

	void PopulateAvailableGamesList(const TArray<FAvailableGame>& AvailableGames);
	void JoinServerAtIndex(const uint32 ServerIndex) const;

	void ShowSessionSearchInProgress() const;
	void HideSessionSearchInProgress() const;

private:
	IMultiplayerGameControls* MultiplayerGameControls;
	
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* FindButton;

	UPROPERTY(meta = (BindWidget))
	UButton* OptionsButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;
	
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* SubmenuSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* HostGameMenuPanel;
	
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* AvailableGamesMenuPanel;

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* OptionsMenuPanel;

	UPROPERTY(meta = (BindWidget))
	UThrobber* SessionSearchInProgress;

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* AvailableGamesList;

	TSubclassOf<UUserWidget> ServerRowClass;
	TSubclassOf<UUserWidget> HostGameMenuClass;

	UFUNCTION()
	void OpenHostMenu();

	UFUNCTION()
	void OpenFindGamesMenu();

	UFUNCTION()
	void OpenOptionsMenu();
	
	UFUNCTION()
	void QuitGame();

	bool IsEveryElementInitialized() const;
	void SetupHostGameMenu() const;
};
