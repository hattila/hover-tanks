// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerRow.h"

#include "MainMenu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UServerRow::Setup(uint32 InIndex, UMainMenu* InMainMenu)
{
	Index = InIndex;
	MainMenu = InMainMenu;

	JoinButton->OnClicked.AddDynamic(this, &UServerRow::OnJoinButtonClicked);
}

void UServerRow::SetServerName(const FString& Name)
{
	ServerName->SetText(FText::FromString(Name));
}

void UServerRow::SetSessionIdString(const FString& InSessionIdString)
{
	SessionIdString->SetText(FText::FromString(InSessionIdString));
}

void UServerRow::SetMapName(const FString& InMapName)
{
	MapName->SetText(FText::FromString(InMapName));
}

void UServerRow::SetGameMode(const FString& InGameMode)
{
	GameMode->SetText(FText::FromString(InGameMode));
}

void UServerRow::SetNumberOfPlayers(const FString& Players)
{
	NumberOfPlayers->SetText(FText::FromString(Players));
}

void UServerRow::SetPing(const FString& InPing)
{
	Ping->SetText(FText::FromString(InPing));
}

void UServerRow::OnJoinButtonClicked()
{
	MainMenu->JoinServerAtIndex(Index);
}
