// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "HoverTanks/Game/HoverTanksGameInstance.h"

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

	bIsFocusable = true;
	
	if (!Success)
	{
		return false;
	};

	if (!ensure(HostButton != nullptr))
	{
		return false;	
	}

	HostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);
	
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

	UE_LOG(LogTemp, Warning, TEXT("Setup done"));
	
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

void UMainMenu::Teardown()
{
	RemoveFromParent();

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);

	PlayerController->bShowMouseCursor = false;
}

void UMainMenu::OpenHostMenu()
{
	// todo going to be a widget switch, now just starts a game

	// Get The GameInstance
	if (!GetWorld())
	{
		return;
	}

	UHoverTanksGameInstance* GameInstance = Cast<UHoverTanksGameInstance>(GetWorld()->GetGameInstance());
	if (GameInstance)
	{
		GameInstance->Host();
	}
	
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
