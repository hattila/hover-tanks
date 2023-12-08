// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTanksGameInstance.h"

#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "HoverTanks/MenuSystem/MainMenu.h"

UHoverTanksGameInstance::UHoverTanksGameInstance(const FObjectInitializer& ObjectInitializer)
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
		FNamedOnlineSession *ExistingSession = SessionInterface->GetNamedSession(TEXT("My Hover Tanks Game"));
		if (ExistingSession != nullptr)
		{
			SessionInterface->DestroySession(TEXT("My Hover Tanks Game"));
			UE_LOG(LogTemp, Warning, TEXT("Destroying existing session"));
			return;
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
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bAllowJoinViaPresence = true;
		SessionSettings.bUseLobbiesIfAvailable = true;
		
		SessionInterface->CreateSession(0, TEXT("My Hover Tanks Game"), SessionSettings);
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

	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete success, Traveling ..."));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	// todo use chosen map
	World->ServerTravel("/Game/HoverTanks/Maps/PrototypeMap?listen");
}

void UHoverTanksGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionComplete was %s"), bWasSuccessful ? TEXT("successful") : TEXT("unsuccessful"));
}

void UHoverTanksGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
}

void UHoverTanksGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
}

/**
 * @see https://www.reddit.com/r/unrealengine/comments/wprvfy/unreal_steam_join_game_setup_how_does_it_work/
 */
void UHoverTanksGameInstance::OnSessionUserInviteAccepted(bool bWasSuccess, int ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->JoinSession(0, NAME_GameSession, InviteResult);	
	}
	
}
