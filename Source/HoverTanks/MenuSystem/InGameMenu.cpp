// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameMenu.h"

#include "Components/Button.h"

bool UInGameMenu::Initialize()
{
	bool Success = Super::Initialize();

	bIsFocusable = true;
	
	if (!Success)
	{
		return false;
	}

	if (!IsEveryElementInitialized())
	{
		return false;
	}

	ResumeGameButton->OnClicked.AddDynamic(this, &UInGameMenu::ResumeGame);
	QuitToMainMenuButton->OnClicked.AddDynamic(this, &UInGameMenu::QuitToMainMenu);
	QuitGameButton->OnClicked.AddDynamic(this, &UInGameMenu::QuitGame);

	return true;
}

void UInGameMenu::Setup()
{
	AddToViewport();
	SetupInputModeGameAndUi();

	bIsOpen = true;
}

void UInGameMenu::Teardown()
{
	Super::Teardown();

	bIsOpen = false;
}

bool UInGameMenu::IsEveryElementInitialized()
{
	if (!ensure(ResumeGameButton != nullptr))
	{
		return false;
	}

	if (!ensure(QuitToMainMenuButton != nullptr))
	{
		return false;
	}

	if (!ensure(QuitGameButton != nullptr))
	{
		return false;
	}

	return true;
}

void UInGameMenu::ResumeGame()
{
	Teardown();
}

void UInGameMenu::QuitToMainMenu()
{
	Teardown();

	// Disconnect from current game
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	// destroy the session
	PlayerController->ConsoleCommand("disconnect");
	
	PlayerController->ClientTravel("/Game/HoverTanks/Maps/MainMenuMap", ETravelType::TRAVEL_Absolute);
}

/**
 * Duplicates method with same name in main menu
 */
void UInGameMenu::QuitGame()
{
	if (!GetWorld())
	{
		return;
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	PlayerController->ConsoleCommand("quit");
}