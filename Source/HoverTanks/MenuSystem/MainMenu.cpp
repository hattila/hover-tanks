// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"

#include "ServerRow.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/Throbber.h"
#include "HoverTanks/Game/HoverTanksGameInstance.h"

UMainMenu::UMainMenu(const FObjectInitializer& ObjectInitializer)
	: UUserWidget(ObjectInitializer),
      MultiplayerGameControls(nullptr),
	  HostButton(nullptr),
	  FindButton(nullptr),
	  QuitButton(nullptr),
	  SubmenuSwitcher(nullptr),
	  HostGameMenu(nullptr),
	  AvailableGamesMenu(nullptr),
      SessionSearchInProgress(nullptr),
      AvailableGamesList(nullptr)
{
	// Super::Construct();

	ConstructorHelpers::FClassFinder<UUserWidget> ServerRowBPClass(TEXT("/Game/HoverTanks/Menu/WBP_ServerRow"));
	if (!ensure(ServerRowBPClass.Class != nullptr))
	{
		return;
	}
	ServerRowClass = ServerRowBPClass.Class;
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

	if (!ensure(FindButton != nullptr))
	{
		return false;	
	}

	FindButton->OnClicked.AddDynamic(this, &UMainMenu::OpenFindGamesMenu);
	
	if (!ensure(QuitButton != nullptr))
	{
		return false;	
	}

	QuitButton->OnClicked.AddDynamic(this, &UMainMenu::QuitGame);

	if (!ensure(AvailableGamesList != nullptr))
	{
		return false;	
	}

	AvailableGamesList->ClearChildren();
	
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

	HideSessionSearchInProgress();
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

void UMainMenu::PopulateAvailableGamesList(const TArray<FString>& ServerNames)
{
	if (!ensure(AvailableGamesList != nullptr))
	{
		UE_LOG(LogTemp, Warning, TEXT("AvailableGamesList is null"));
		return;
	}
	
	AvailableGamesList->ClearChildren();

	uint32 i = 0;
	for (const FString& ServerName : ServerNames)
	{
		UServerRow* ServerRow = CreateWidget<UServerRow>(this, ServerRowClass);
		if (!ensure(ServerRow != nullptr))
		{
			return;
		}

		ServerRow->SetServerName(*ServerName);
		ServerRow->SetNumberOfPlayers(TEXT("1 / 12"));
		ServerRow->Setup(i, this);
		++i;

		AvailableGamesList->AddChild(ServerRow);
	}
	
}

void UMainMenu::JoinServerAtIndex(uint32 ServerIndex)
{
	if (AvailableGamesList == nullptr)
	{
		return;
	}

	UServerRow* ServerRow = Cast<UServerRow>(AvailableGamesList->GetChildAt(ServerIndex));
	if (ServerRow && MultiplayerGameControls)
	{
		MultiplayerGameControls->JoinAvailableGame(ServerIndex);
	}
	
}

void UMainMenu::ShowSessionSearchInProgress()
{
	SessionSearchInProgress->SetVisibility(ESlateVisibility::Visible);
}

void UMainMenu::HideSessionSearchInProgress()
{
	SessionSearchInProgress->SetVisibility(ESlateVisibility::Hidden);
}

void UMainMenu::OpenHostMenu()
{
	if (!GetWorld())
	{
		return;
	}

	if (SubmenuSwitcher)
	{
		SubmenuSwitcher->SetActiveWidget(HostGameMenu);
	}
}

void UMainMenu::OpenFindGamesMenu()
{
	if (SubmenuSwitcher)
	{
		SubmenuSwitcher->SetActiveWidget(AvailableGamesMenu);
	}

	if (MultiplayerGameControls)
	{
		MultiplayerGameControls->RefreshServerList();
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
