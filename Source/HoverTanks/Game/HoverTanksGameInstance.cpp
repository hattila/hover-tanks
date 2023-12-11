// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTanksGameInstance.h"

#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "HoverTanks/MenuSystem/MainMenu.h"

UHoverTanksGameInstance::UHoverTanksGameInstance(const FObjectInitializer& ObjectInitializer): MainMenu(nullptr)
{
	// load MainMenuClass
	static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuBPClass(TEXT("/Game/HoverTanks/Menu/WBP_MainMenu"));
	if (!ensure(MainMenuBPClass.Class != nullptr))
	{
		return;
	}

	MainMenuClass = MainMenuBPClass.Class;
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

	// Create a MainMenu widget and add it to the viewport
	MainMenu = CreateWidget<UMainMenu>(this, MainMenuClass);
	if (MainMenu == nullptr)
	{
		return;
	}

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

	SessionInterface->JoinSession(0, GHover_Tanks_Session_Name, SessionSearch->SearchResults[Index]);
}

void UHoverTanksGameInstance::StartCreateSession()
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;

		IOnlineSubsystem::Get()->GetSubsystemName() == "NULL"
			? SessionSettings.bIsLANMatch = true
			: SessionSettings.bIsLANMatch = false;

		SessionSettings.NumPublicConnections = 10;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		// SessionSettings.bAllowJoinInProgress = true;
		// SessionSettings.bAllowJoinViaPresence = true;
		SessionSettings.bUseLobbiesIfAvailable = true;
		
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

	const FString MapName = HostGameSettings.MapName != "" ? HostGameSettings.MapName : "PrototypeMap";
	FString ServerTravelURL = FString::Printf(TEXT("/Game/HoverTanks/Maps/%s?listen"), *MapName);

	if (HostGameSettings.GameModeName != "")
	{
		ServerTravelURL.Append(FString::Printf(TEXT("&game=%s"), *HostGameSettings.GameModeName));
	}

	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete success, Traveling with string ... %s"), *ServerTravelURL);
	
	World->ServerTravel(ServerTravelURL, true);
}

void UHoverTanksGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionComplete was %s"), bWasSuccessful ? TEXT("successful") : TEXT("unsuccessful"));
}

void UHoverTanksGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		TArray<FString> ServerNames;
		// ServerNames.Add("Test 1");
		// ServerNames.Add("Tester 2");
		// ServerNames.Add("T 3");
		
		for (FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Session found: %s"), *SearchResult.GetSessionIdStr());

			ServerNames.Add(SearchResult.GetSessionIdStr());
		}

		MainMenu->PopulateAvailableGamesList(ServerNames);
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

	PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
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
