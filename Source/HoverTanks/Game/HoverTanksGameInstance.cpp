// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTanksGameInstance.h"

#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"

UHoverTanksGameInstance::UHoverTanksGameInstance(const FObjectInitializer& ObjectInitializer)
{
	
}

void UHoverTanksGameInstance::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("Initializing HoverTanksGameInstance"));
	
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();

	SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UHoverTanksGameInstance::OnCreateSessionComplete);
		SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UHoverTanksGameInstance::OnDestroySessionComplete);
		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UHoverTanksGameInstance::OnFindSessionsComplete);
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UHoverTanksGameInstance::OnJoinSessionComplete);
	}
	
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

void UHoverTanksGameInstance::Join(const FString& Address)
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (PlayerController)
	{
		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
}

void UHoverTanksGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("OnCreateSessionComplete failed"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete success, Traveling ..."));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	World->ServerTravel("/Game/HoverTanks/Maps/PrototypeMap?listen");
}

void UHoverTanksGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UHoverTanksGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
}

void UHoverTanksGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
}
