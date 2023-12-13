// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"

#include "HostGameMenu.h"
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
      OptionsButton(nullptr),
	  QuitButton(nullptr),
	  SubmenuSwitcher(nullptr),
	  HostGameMenuPanel(nullptr),
	  AvailableGamesMenuPanel(nullptr),
      OptionsMenuPanel(nullptr),
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

	ConstructorHelpers::FClassFinder<UUserWidget> HostGameMenuBPClass(TEXT("/Game/HoverTanks/Menu/WBP_HostGameMenu"));
	if (!ensure(HostGameMenuBPClass.Class != nullptr))
	{
		return;
	}
	HostGameMenuClass = HostGameMenuBPClass.Class;
}

bool UMainMenu::Initialize()
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

	HostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);
	FindButton->OnClicked.AddDynamic(this, &UMainMenu::OpenFindGamesMenu);
	OptionsButton->OnClicked.AddDynamic(this, &UMainMenu::OpenOptionsMenu);
	QuitButton->OnClicked.AddDynamic(this, &UMainMenu::QuitGame);

	AvailableGamesList->ClearChildren();
	
	return true;
}

void UMainMenu::Setup(IMultiplayerGameControls* InMultiplayerGameControls)
{
	AddToViewport();

	SetupInputModeUIOnly();
	HideSessionSearchInProgress();

	MultiplayerGameControls = InMultiplayerGameControls;

	SetupHostGameMenu();
}

void UMainMenu::Teardown()
{
	RemoveFromParent();

	SetInputModeGameOnly();
}

void UMainMenu::PopulateAvailableGamesList(const TArray<FAvailableGame>& AvailableGames)
{
	if (!ensure(AvailableGamesList != nullptr))
	{
		UE_LOG(LogTemp, Warning, TEXT("AvailableGamesList is null"));
		return;
	}
	
	AvailableGamesList->ClearChildren();

	uint32 i = 0;
	for (const FAvailableGame& AvailableGame : AvailableGames)
	{
		UServerRow* ServerRow = CreateWidget<UServerRow>(this, ServerRowClass);
		if (!ensure(ServerRow != nullptr))
		{
			return;
		}

		ServerRow->SetServerName(*AvailableGame.ServerName);
		ServerRow->SetSessionIdString(*AvailableGame.SessionIdString);

		ServerRow->SetMapName(*AvailableGame.MapName);
		ServerRow->SetGameMode(*AvailableGame.GameModeName);

		ServerRow->SetNumberOfPlayers(FString::Printf(TEXT("%d/%d"), AvailableGame.CurrentPlayers, AvailableGame.MaxPlayers));
		ServerRow->SetPing(*AvailableGame.Ping);

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
		SubmenuSwitcher->SetActiveWidget(HostGameMenuPanel);
	}
}

void UMainMenu::OpenFindGamesMenu()
{
	if (SubmenuSwitcher)
	{
		SubmenuSwitcher->SetActiveWidget(AvailableGamesMenuPanel);
	}

	if (MultiplayerGameControls)
	{
		MultiplayerGameControls->RefreshServerList();
	}
}

void UMainMenu::OpenOptionsMenu()
{
	if (SubmenuSwitcher)
	{
		SubmenuSwitcher->SetActiveWidget(OptionsMenuPanel);
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

bool UMainMenu::IsEveryElementInitialized() const
{
	if (!ensure(HostButton != nullptr))
	{
		return false;	
	}

	if (!ensure(FindButton != nullptr))
	{
		return false;	
	}
	
	if (!ensure(OptionsButton != nullptr))
	{
		return false;	
	}

	if (!ensure(QuitButton != nullptr))
	{
		return false;	
	}

	if (!ensure(AvailableGamesList != nullptr))
	{
		return false;	
	}

	return true;
}

void UMainMenu::SetupInputModeUIOnly()
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

void UMainMenu::SetInputModeGameOnly()
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

void UMainMenu::SetupHostGameMenu()
{
	if (HostGameMenuPanel == nullptr || !HostGameMenuClass || MultiplayerGameControls == nullptr)
	{
		return;
	}

	/**
	 * Creating the HostGameMenu widget here works, but I cannot figure out how to position it correctly.
	 * So until then, I'm going to create it in the HostGameMenuPanel in the editor, and add the MultiplayerGameControls here.
	 */
	
	// Create HostGameMenu widget
	// UHostGameMenu* HostGameMenu = CreateWidget<UHostGameMenu>(this, HostGameMenuClass);
	// HostGameMenu->SetMultiplayerGameControls(MultiplayerGameControls);
	//
	// HostGameMenuPanel->AddChild(HostGameMenu);
	//
	// FVector2d DesiredSize = FVector2d(2540, 2000);
	// HostGameMenu->SetDesiredSizeInViewport(DesiredSize);
	//
	// UE_LOG(LogTemp, Warning, TEXT("HostGameMenu created, with size %f, %f"), DesiredSize.X, DesiredSize.Y);

	TArray<UWidget*> HostGameMenuChildren;
	HostGameMenuChildren = HostGameMenuPanel->GetAllChildren();
	for (UWidget* HostGameMenuChild : HostGameMenuChildren)
	{
		if (HostGameMenuChild->GetClass() == HostGameMenuClass)
		{
			UHostGameMenu* HostGameMenu = Cast<UHostGameMenu>(HostGameMenuChild);
			if (HostGameMenu)
			{
				HostGameMenu->SetMultiplayerGameControls(MultiplayerGameControls);
			}
		}
	}
}