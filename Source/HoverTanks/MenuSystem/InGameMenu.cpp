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
	SetupInputModeUIOnly();
}

void UInGameMenu::Teardown()
{
	RemoveFromParent();
	SetInputModeGameOnly();
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

/**
 * Duplicates method with same name in main menu
 */
void UInGameMenu::SetupInputModeUIOnly()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	
	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(true);
}

/**
 * Duplicates method with same name in main menu
 */
void UInGameMenu::SetInputModeGameOnly()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);

	PlayerController->bShowMouseCursor = false;
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