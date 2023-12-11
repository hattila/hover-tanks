// Fill out your copyright notice in the Description page of Project Settings.


#include "HostGameMenu.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "HoverTanks/Game/HoverTanksGameInstance.h"

bool UHostGameMenu::Initialize()
{
	bool Success = Super::Initialize();
	bIsFocusable = true;
	
	if (!Success)
	{
		return false;
	};

	if (!ensure(MapSelection != nullptr))
	{
		return false;	
	}

	MapSelection->AddOption(TEXT("PrototypeMap"));
	MapSelection->AddOption(TEXT("Map 2"));
	MapSelection->AddOption(TEXT("Map 3"));
	MapSelection->SetSelectedIndex(0);

	MapSelection->OnSelectionChanged.AddDynamic(this, &UHostGameMenu::OnMapSelectionChanged);

	if (!ensure(GameModeSelection != nullptr))
	{
		return false;	
	}

	GameModeSelection->AddOption(TEXT("Deathmatch"));
	GameModeSelection->AddOption(TEXT("Team Deathmatch"));
	GameModeSelection->AddOption(TEXT("Destruction Of The Ancients"));
	GameModeSelection->SetSelectedIndex(0);

	GameModeSelection->OnSelectionChanged.AddDynamic(this, &UHostGameMenu::OnGameModeSelectionChanged);
	
	if (!ensure(StartGame != nullptr))
	{
		return false;
	}
	StartGame->OnClicked.AddDynamic(this, &UHostGameMenu::OnStartGameClicked);
	

	return true;
}

void UHostGameMenu::OnMapSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UE_LOG(LogTemp, Warning, TEXT("Selected map: %s"), *SelectedItem);
}

void UHostGameMenu::OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UE_LOG(LogTemp, Warning, TEXT("Selected game mode: %s"), *SelectedItem);
}

void UHostGameMenu::OnStartGameClicked()
{
	UHoverTanksGameInstance* GameInstance = Cast<UHoverTanksGameInstance>(GetWorld()->GetGameInstance());
	if (GameInstance)
	{
		GameInstance->Host();
	}
}
