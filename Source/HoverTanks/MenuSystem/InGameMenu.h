// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OpenableMenu.h"
#include "InGameMenu.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UInGameMenu : public UOpenableMenu
{
	GENERATED_BODY()
public:
	virtual bool Initialize() override;
	virtual void Setup() override;
	virtual void Teardown() override;

	bool IsOpen() const { return bIsOpen; }

private:
	UPROPERTY(meta = (BindWidget))
	UButton* ResumeGameButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* QuitToMainMenuButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* QuitGameButton;

	bool bIsOpen = false;

	virtual bool IsEveryElementInitialized() const override;

	UFUNCTION()
	void ResumeGame();
	
	UFUNCTION()
	void QuitToMainMenu();

	UFUNCTION()
	void QuitGame();
};
