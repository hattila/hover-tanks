// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InGameMenu.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UInGameMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	virtual bool Initialize() override;
	void Setup();
	void Teardown();

	bool IsOpen() const { return bIsOpen; }

private:
	UPROPERTY(meta = (BindWidget))
	UButton* ResumeGameButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* QuitToMainMenuButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* QuitGameButton;

	bool bIsOpen = false;

	bool IsEveryElementInitialized();
	void SetupInputModeUIOnly();
	void SetInputModeGameOnly();

	UFUNCTION()
	void ResumeGame();
	
	UFUNCTION()
	void QuitToMainMenu();

	UFUNCTION()
	void QuitGame();
};
