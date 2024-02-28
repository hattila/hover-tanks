// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OpenableMenu.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"


class IHTGI_MultiplayerGameControlsInterface;
struct FAvailableGame;

class UThrobber;
class UServerRow;
class UHTGameInstance;
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
	void SetMultiplayerGameControls(IHTGI_MultiplayerGameControlsInterface* InMultiplayerGameControls) { MultiplayerGameControls = InMultiplayerGameControls; }

	virtual bool Initialize() override;
	virtual void Setup() override;
	virtual void Teardown() override;

	void PopulateAvailableGamesList(const TArray<FAvailableGame>& AvailableGames);
	void JoinServerAtIndex(const uint32 ServerIndex) const;

	void ShowSessionSearchInProgress() const;
	void HideSessionSearchInProgress() const;

protected:
	virtual bool IsEveryElementInitialized() const override;
	
private:
	IHTGI_MultiplayerGameControlsInterface* MultiplayerGameControls = nullptr;
	
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton = nullptr;
	
	UPROPERTY(meta = (BindWidget))
	UButton* FindButton = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* OptionsButton = nullptr;
	
	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton = nullptr;
	
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* SubmenuSwitcher = nullptr;
	
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* HostGameMenuPanel = nullptr;
	
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* AvailableGamesMenuPanel = nullptr;

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* OptionsMenuPanel = nullptr;

	UPROPERTY(meta = (BindWidget))
	UThrobber* SessionSearchInProgress = nullptr;

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* AvailableGamesList = nullptr;

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

	void SetupHostGameMenu() const;
};
