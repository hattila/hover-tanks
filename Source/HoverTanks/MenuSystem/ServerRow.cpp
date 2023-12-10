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

void UServerRow::SetNumberOfPlayers(const FString& Players)
{
	NumberOfPlayers->SetText(FText::FromString(Players));
}

void UServerRow::OnJoinButtonClicked()
{
	MainMenu->JoinServerAtIndex(Index);
}
