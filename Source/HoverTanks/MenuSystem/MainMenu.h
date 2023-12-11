// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

class UThrobber;
class UServerRow;
class UHoverTanksGameInstance;
class UWidgetSwitcher;
class UButton;

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenu(const FObjectInitializer& ObjectInitializer);

	UFUNCTION()
	virtual bool Initialize() override;
	void Setup();
	void Teardown();

	void PopulateAvailableGamesList(const TArray<FString>& ServerNames);
	void JoinServerAtIndex(uint32 ServerIndex);

	void ShowSessionSearchInProgress();
	void HideSessionSearchInProgress();

private:
	// UHoverTanksGameInstance* GameInstance;
	
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* FindButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;
	
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* SubmenuSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* HostGameMenu;
	
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* AvailableGamesMenu;

	UPROPERTY(meta = (BindWidget))
	UThrobber* SessionSearchInProgress;

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* AvailableGamesList;

	TSubclassOf<UUserWidget> ServerRowClass;

	UFUNCTION()
	void OpenHostMenu();

	UFUNCTION()
	void OpenFindGamesMenu();
	
	UFUNCTION()
	void QuitGame();
};
