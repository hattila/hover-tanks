// Fill out your copyright notice in the Description page of Project Settings.


#include "OptionsMenu.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "GameFramework/GameUserSettings.h"

bool UOptionsMenu::Initialize()
{
	bool Success = Super::Initialize();

	if (!Success)
	{
		return false;
	}

	if (!IsEveryElementInitialized())
	{
		return false;
	}

	GameUserSettings = UGameUserSettings::GetGameUserSettings();

	GraphicsSelection->AddOption(TEXT("Low"));
	GraphicsSelection->AddOption(TEXT("Medium"));
	GraphicsSelection->AddOption(TEXT("High"));
	GraphicsSelection->AddOption(TEXT("Epic"));
	GraphicsSelection->SetSelectedIndex(3);

	GraphicsSelection->OnSelectionChanged.AddDynamic(this, &UOptionsMenu::OnGraphicsSelectionChanged);
	
	ResolutionSelection->AddOption(TEXT("1280x720"));
	ResolutionSelection->AddOption(TEXT("1600x900"));
	ResolutionSelection->AddOption(TEXT("1920x1080"));
	ResolutionSelection->AddOption(TEXT("2560x1440"));
	ResolutionSelection->AddOption(TEXT("3840x2160"));
	ResolutionSelection->SetSelectedIndex(3);

	ResolutionSelection->OnSelectionChanged.AddDynamic(this, &UOptionsMenu::OnResolutionSelectionChanged);

	SaveButton->OnClicked.AddDynamic(this, &UOptionsMenu::OnSaveButtonClicked);

	
	return true;
}

bool UOptionsMenu::IsEveryElementInitialized() const
{
	if (!ensure(GraphicsSelection != nullptr))
	{
		return false;
	}

	if (!ensure(ResolutionSelection != nullptr))
	{
		return false;
	}

	if (!ensure(SaveButton != nullptr))
	{
		return false;
	}

	return true;
}

void UOptionsMenu::OnGraphicsSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UE_LOG(LogTemp, Warning, TEXT("Selected graphics: %s"), *SelectedItem);

	if (!ensure(GameUserSettings != nullptr))
	{
		return;
	}

	if (SelectedItem == "Low")
	{
		GameUserSettings->SetOverallScalabilityLevel(0);
	}

	if (SelectedItem == "Medium")
	{
		GameUserSettings->SetOverallScalabilityLevel(1);
	}

	if (SelectedItem == "High")
	{
		GameUserSettings->SetOverallScalabilityLevel(2);
	}

	if (SelectedItem == "Epic")
	{
		GameUserSettings->SetOverallScalabilityLevel(3);
	}
}

void UOptionsMenu::OnResolutionSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UE_LOG(LogTemp, Warning, TEXT("Selected resolution: %s"), *SelectedItem);
}

void UOptionsMenu::OnSaveButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Save button clicked."));

	GameUserSettings->ApplySettings(false);
}

