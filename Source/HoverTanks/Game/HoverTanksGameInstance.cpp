// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTanksGameInstance.h"

#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "HoverTanks/MenuSystem/MainMenu.h"
#include "HoverTanks/UI/ToasterWidget.h"
#include "Kismet/GameplayStatics.h"

UHoverTanksGameInstance::UHoverTanksGameInstance(const FObjectInitializer& ObjectInitializer): MainMenu(nullptr)
{
	// load MainMenuClass
	static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuBPClass(TEXT("/Game/HoverTanks/Menu/WBP_MainMenu"));
	if (!ensure(MainMenuBPClass.Class != nullptr))
	{
		return;
	}

	MainMenuClass = MainMenuBPClass.Class;

	// load ToasterWidgetClass
	static ConstructorHelpers::FClassFinder<UUserWidget> ToasterWidgetBPClass(TEXT("/Game/HoverTanks/UI/WBP_ToasterWidget"));
	if (!ensure(ToasterWidgetBPClass.Class != nullptr))
	{
		return;
	}

	ToasterWidgetClass = ToasterWidgetBPClass.Class;
}

void UHoverTanksGameInstance::Init()
{
	Super::Init(); // without this call, widgets cannot initialize and the menu will not show
	
	// UE_LOG(LogTemp, Warning, TEXT("Initializing HoverTanksGameInstance"));
	
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();

	SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UHoverTanksGameInstance::OnCreateSessionComplete);
		SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UHoverTanksGameInstance::OnDestroySessionComplete);
		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UHoverTanksGameInstance::OnFindSessionsComplete);
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UHoverTanksGameInstance::OnJoinSessionComplete);

		SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UHoverTanksGameInstance::OnSessionUserInviteAccepted);
	}
	
}

void UHoverTanksGameInstance::ShowMainMenu()
{
	if (MainMenuClass == nullptr)
	{
		return;
	}

	MainMenu = CreateWidget<UMainMenu>(this, MainMenuClass);
	if (MainMenu == nullptr)
	{
		return;
	}

	MainMenu->SetMultiplayerGameControls(this); // !
	MainMenu->Setup();
}

void UHoverTanksGameInstance::Host()
{
	if (SessionInterface.IsValid())
	{
		FNamedOnlineSession *ExistingSession = SessionInterface->GetNamedSession(GHover_Tanks_Session_Name);
		if (ExistingSession != nullptr)
		{
			SessionInterface->DestroySession(GHover_Tanks_Session_Name);
			UE_LOG(LogTemp, Warning, TEXT("Destroying existing session"));
			// return;
		}
		
		StartCreateSession();
	}
}

void UHoverTanksGameInstance::Join(const FString& Address)
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (PlayerController)
	{
		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
}

/**
 * Console command
 * @param InInputMode - "GameAndUI" or "GameOnly" or "UIOnly"
 */
void UHoverTanksGameInstance::InputMode(const FString& InInputMode)
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputMode cmd: PlayerController not found"));
		return;
	}

	if (InInputMode == "GameAndUI")
	{
		const FInputModeGameAndUI InputModeData;
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = true;
		return;
	}

	if (InInputMode == "GameOnly")
	{
		const FInputModeGameOnly InputModeData;
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = false;
		return;
	}

	if (InInputMode == "UIOnly")
	{
		const FInputModeUIOnly InputModeData;
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = true;
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("InputMode: given input mode does not exist: %s, try GameOnly, GameAndUI or UIOnly"), *InInputMode);
}

void UHoverTanksGameInstance::HostGame(const FHostGameSettings& InHostGameSettings)
{
	HostGameSettings.MapName = InHostGameSettings.MapName;
	HostGameSettings.GameModeName = InHostGameSettings.GameModeName;
	HostGameSettings.MaxPlayers = InHostGameSettings.MaxPlayers;

	Host();
}

void UHoverTanksGameInstance::RefreshServerList()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Searching for sessions ..."));
		
		// SessionSearch->bIsLanQuery = true;
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
		if (MainMenu)
		{
			MainMenu->ShowSessionSearchInProgress();
		}
	}

	AddToastMessage(TEXT("Finding Servers"), true);
}

void UHoverTanksGameInstance::JoinAvailableGame(uint32 Index)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	if (!SessionSearch.IsValid())
	{
		return;
	}

	// teardown menu?
	if (MainMenu)
	{
		MainMenu->Teardown();
	}

	if (GEngine)
	{
		FString Message = FString::Printf(TEXT("Joining game of %s"), *SessionSearch->SearchResults[Index].Session.OwningUserName);
		GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Green, *Message);	
	}

	SessionInterface->JoinSession(0, GHover_Tanks_Session_Name, SessionSearch->SearchResults[Index]);

	AddToastMessage(FString::Printf(TEXT("Joining game of %s"), *SessionSearch->SearchResults[Index].Session.OwningUserName), true);
}

void UHoverTanksGameInstance::AddToastMessage(const FString& String, const bool bShowLoading)
{
	UToasterWidget* ToasterWidget = CreateWidget<UToasterWidget>(this, ToasterWidgetClass);
	if (ToasterWidget)
	{
		ToasterWidget->SetAnchorsInViewport(FAnchors(0.1f, 0.85f, 0.9f, 0.9f));
		ToasterWidget->SetMessage(String);
		ToasterWidget->ShowLoadingInProgress(bShowLoading);
		
		ToasterWidget->AddToViewport();
	}
}

void UHoverTanksGameInstance::StartCreateSession()
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;

		const bool bIsLanGame = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
		
		// IOnlineSubsystem::Get()->GetSubsystemName() == "NULL"
		// 	? SessionSettings.bIsLANMatch = true
		// 	: SessionSettings.bIsLANMatch = false;

		SessionSettings.bIsLANMatch = bIsLanGame;
		SessionSettings.NumPublicConnections = 10;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		SessionSettings.bAllowJoinInProgress = true;
		// SessionSettings.bAllowJoinViaPresence = true;
		SessionSettings.bUseLobbiesIfAvailable = true;

		// add MAPNAME to SessionSettings
		if (HostGameSettings.MapName != "")
		{
			SessionSettings.Set(TEXT("MAPNAME"), HostGameSettings.MapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		}

		// add GAMEMODE to SessionSettings
		if (HostGameSettings.GameModeName != "")
		{
			SessionSettings.Set(TEXT("GAMEMODE"), HostGameSettings.GameModeName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		}
		
		if (GEngine)
		{
			FString Message = FString::Printf(TEXT("Creating a%s session"), SessionSettings.bIsLANMatch ? TEXT(" LAN") : TEXT("n ONLINE"));
			GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Green, *Message);	
		}
		
		SessionInterface->CreateSession(0, GHover_Tanks_Session_Name, SessionSettings);
	}
}

void UHoverTanksGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("OnCreateSessionComplete failed"));
		return;
	}

	if (MainMenu != nullptr)
	{
		MainMenu->Teardown();
	}

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	const FString MapName = HostGameSettings.MapName != "" ? HostGameSettings.MapName.Append(TEXT("Map")) : "DesertRampsMap";
	FString ServerTravelURL = FString::Printf(TEXT("/Game/HoverTanks/Maps/%s?listen"), *MapName);

	/**
	 * Set the Game parameter if GameMode is chosen
	 */
	if (HostGameSettings.GameModeName != "")
	{
		// const FString GameModePath = FString::Printf(TEXT("HoverTanks./Script/HoverTanks/Game/GameModes/%s"), *HostGameSettings.GameModeName);
		const FString GameModePath = FString::Printf(TEXT("HoverTanks.Game.GameModes.%sGameMode"), *HostGameSettings.GameModeName); // todo class exists check
		ServerTravelURL.Append(FString::Printf(TEXT("?Game=%s"), *GameModePath));
	}

	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete success, Traveling with string ... %s"), *ServerTravelURL);

	if (GEngine)
	{
		FString Message = FString::Printf(TEXT("Starting up server with params: %s"), *ServerTravelURL);
		GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Green, *Message);
	}

	/**
	 * ServerTravel or OpenLevel
	 */
	// World->ServerTravel(ServerTravelURL, true); // eg "/Game/HoverTanks/Maps/PrototypeMap?listen
	UGameplayStatics::OpenLevel(World, FName(*MapName), true, ServerTravelURL);
}

void UHoverTanksGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionComplete was %s"), bWasSuccessful ? TEXT("successful") : TEXT("unsuccessful"));
}

void UHoverTanksGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	// add an on screen debug message
	if (GEngine)
	{
		FString Message = FString::Printf(TEXT("FindSessionsComplete, was %s"), bWasSuccessful ? TEXT("successful") : TEXT("unsuccessful"));
		GEngine->AddOnScreenDebugMessage(0, 5.f, bWasSuccessful ? FColor::Green : FColor::Red, *Message);
	}
	
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		TArray<FString> ServerNames;
		// ServerNames.Add("Test 1");
		// ServerNames.Add("Tester 2");
		// ServerNames.Add("T 3");

		TArray<FAvailableGame> AvailableGames;
		
		for (FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Session found: %s"), *SearchResult.GetSessionIdStr());

			ServerNames.Add(SearchResult.GetSessionIdStr());

			FAvailableGame AvailableGame;
			AvailableGame.ServerName = SearchResult.Session.OwningUserName;
			AvailableGame.ServerName.Append(FString::Printf(TEXT("'s game")));
			AvailableGame.SessionIdString = SearchResult.GetSessionIdStr();
			
			AvailableGame.MapName = SearchResult.Session.SessionSettings.Settings.FindRef("MAPNAME").Data.ToString();
			AvailableGame.GameModeName = SearchResult.Session.SessionSettings.Settings.FindRef("GAMEMODE").Data.ToString();
			
			AvailableGame.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
			AvailableGame.CurrentPlayers = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;
			AvailableGame.Ping = FString::Printf(TEXT("%i ms"), SearchResult.PingInMs);

			AvailableGames.Add(AvailableGame);
		}

		MainMenu->PopulateAvailableGamesList(AvailableGames);
		MainMenu->HideSessionSearchInProgress();

		UE_LOG(LogTemp, Warning, TEXT("FindSessionsComplete"));
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Session not found"));
	}
}

void UHoverTanksGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FString Address;
	if (!SessionInterface->GetResolvedConnectString(SessionName, Address))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not get resolve connect string %s"), *SessionName.ToString());
		return;
	}

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	if (GEngine)
	{
		FString Message = FString::Printf(TEXT("OnJoinSessionComplete, was successful, ClientTravel to %s"), *Address);
		GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Green, *Message);
	}

	PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Relative);
}

/**
 * @see https://www.reddit.com/r/unrealengine/comments/wprvfy/unreal_steam_join_game_setup_how_does_it_work/
 */
void UHoverTanksGameInstance::OnSessionUserInviteAccepted(bool bWasSuccess, int ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Attempting to join with User Invite"));
	
	if (SessionInterface.IsValid())
	{
		SessionInterface->JoinSession(0, GHover_Tanks_Session_Name, InviteResult);	
	}
	
}
