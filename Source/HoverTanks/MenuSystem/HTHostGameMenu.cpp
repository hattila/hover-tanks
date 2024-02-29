// Fill out your copyright notice in the Description page of Project Settings.


#include "HTHostGameMenu.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "HoverTanks/Game/HTGameInstance.h"

bool UHTHostGameMenu::Initialize()
{
	const bool Success = Super::Initialize();
	bIsFocusable = true;
	
	if (!Success)
	{
		return false;
	};

	if (!ensure(MapSelection != nullptr))
	{
		return false;	
	}

	MapSelection->AddOption(TEXT("Prototype"));
	MapSelection->AddOption(TEXT("SnowValley"));
	MapSelection->AddOption(TEXT("DesertRamps"));
	MapSelection->SetSelectedIndex(2);

	MapSelection->OnSelectionChanged.AddDynamic(this, &UHTHostGameMenu::OnMapSelectionChanged);

	if (!ensure(GameModeSelection != nullptr))
	{
		return false;	
	}

	GameModeSelection->AddOption(TEXT("DeathMatch"));
	GameModeSelection->AddOption(TEXT("TeamDeathMatch"));
	GameModeSelection->AddOption(TEXT("DestructionOfTheAncients"));
	GameModeSelection->SetSelectedIndex(0);

	GameModeSelection->OnSelectionChanged.AddDynamic(this, &UHTHostGameMenu::OnGameModeSelectionChanged);
	
	if (!ensure(StartGame != nullptr))
	{
		return false;
	}
	StartGame->OnClicked.AddDynamic(this, &UHTHostGameMenu::OnStartGameClicked);
	

	return true;
}

void UHTHostGameMenu::OnMapSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UE_LOG(LogTemp, Warning, TEXT("Selected map: %s"), *SelectedItem);
}

void UHTHostGameMenu::OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UE_LOG(LogTemp, Warning, TEXT("Selected game mode: %s"), *SelectedItem);
}

void UHTHostGameMenu::OnStartGameClicked()
{
	if (MultiplayerGameControls)
	{
		FHostGameSettings HostGameSettings;
		HostGameSettings.MapName = MapSelection->GetSelectedOption();
		HostGameSettings.GameModeName = GameModeSelection->GetSelectedOption();
		
		MultiplayerGameControls->HostGame(HostGameSettings);
	}
}
