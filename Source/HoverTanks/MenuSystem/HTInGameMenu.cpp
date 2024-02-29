// Fill out your copyright notice in the Description page of Project Settings.


#include "HTInGameMenu.h"

#include "Components/Button.h"

bool UHTInGameMenu::Initialize()
{
	const bool Success = Super::Initialize();

	bIsFocusable = true;
	
	if (!Success)
	{
		return false;
	}

	if (!IsEveryElementInitialized())
	{
		return false;
	}

	ResumeGameButton->OnClicked.AddDynamic(this, &UHTInGameMenu::ResumeGame);
	QuitToMainMenuButton->OnClicked.AddDynamic(this, &UHTInGameMenu::QuitToMainMenu);
	QuitGameButton->OnClicked.AddDynamic(this, &UHTInGameMenu::QuitGame);

	return true;
}

void UHTInGameMenu::Setup()
{
	AddToViewport();
	SetupInputModeGameAndUi();

	bIsOpen = true;
}

void UHTInGameMenu::Teardown()
{
	Super::Teardown();

	bIsOpen = false;
}

bool UHTInGameMenu::IsEveryElementInitialized() const
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

void UHTInGameMenu::ResumeGame()
{
	Teardown();
}

void UHTInGameMenu::QuitToMainMenu()
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
void UHTInGameMenu::QuitGame()
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