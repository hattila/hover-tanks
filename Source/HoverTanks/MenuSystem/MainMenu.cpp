// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"

UMainMenu::UMainMenu(const FObjectInitializer& ObjectInitializer)
	: UUserWidget(ObjectInitializer),
	  HostButton(nullptr),
	  FindButton(nullptr),
	  QuitButton(nullptr),
	  SubmenuSwitcher(nullptr),
	  HostGameMenu(nullptr),
	  AvailableGamesList(nullptr)
{
	// Super::Construct();
}

bool UMainMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success)
	{
		return false;
	};
	
	if (!ensure(QuitButton != nullptr))
	{
		return false;	
	}

	QuitButton->OnClicked.AddDynamic(this, &UMainMenu::QuitGame);
	

	return true;
}

void UMainMenu::Setup()
{
	AddToViewport();

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(this->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(true);
}

void UMainMenu::QuitGame()
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
