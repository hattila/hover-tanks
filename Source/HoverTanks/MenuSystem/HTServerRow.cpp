// Fill out your copyright notice in the Description page of Project Settings.


#include "HTServerRow.h"

#include "HTMainMenu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UHTServerRow::Setup(uint32 InIndex, UHTMainMenu* InMainMenu)
{
	Index = InIndex;
	MainMenu = InMainMenu;

	JoinButton->OnClicked.AddDynamic(this, &UHTServerRow::OnJoinButtonClicked);
}

void UHTServerRow::SetServerName(const FString& Name)
{
	ServerName->SetText(FText::FromString(Name));
}

void UHTServerRow::SetSessionIdString(const FString& InSessionIdString)
{
	SessionIdString->SetText(FText::FromString(InSessionIdString));
}

void UHTServerRow::SetMapName(const FString& InMapName)
{
	MapName->SetText(FText::FromString(InMapName));
}

void UHTServerRow::SetGameMode(const FString& InGameMode)
{
	GameMode->SetText(FText::FromString(InGameMode));
}

void UHTServerRow::SetNumberOfPlayers(const FString& Players)
{
	NumberOfPlayers->SetText(FText::FromString(Players));
}

void UHTServerRow::SetPing(const FString& InPing)
{
	Ping->SetText(FText::FromString(InPing));
}

void UHTServerRow::OnJoinButtonClicked()
{
	MainMenu->JoinServerAtIndex(Index);
}
