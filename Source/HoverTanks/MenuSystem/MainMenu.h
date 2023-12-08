// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

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
	bool Initialize() override;
	void Setup();
	void Teardown();

private:
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
	UPanelWidget* AvailableGamesList;

	UFUNCTION()
	void OpenHostMenu();
	
	UFUNCTION()
	void QuitGame();
};
