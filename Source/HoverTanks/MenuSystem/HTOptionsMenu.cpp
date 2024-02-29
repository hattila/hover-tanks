// Fill out your copyright notice in the Description page of Project Settings.


#include "HTOptionsMenu.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "GameFramework/GameUserSettings.h"

bool UHTOptionsMenu::Initialize()
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

	SetupGraphicsSelection();
	SetupResolutionSelection();
	SetupWindowModeSelection();

	SaveButton->OnClicked.AddDynamic(this, &UHTOptionsMenu::OnSaveButtonClicked);

	return true;
}

bool UHTOptionsMenu::IsEveryElementInitialized() const
{
	if (!ensure(GraphicsSelection != nullptr))
	{
		return false;
	}

	if (!ensure(ResolutionSelection != nullptr))
	{
		return false;
	}

	if (!ensure(WindowModeSelection != nullptr))
	{
		return false;
	}

	if (!ensure(SaveButton != nullptr))
	{
		return false;
	}

	return true;
}

void UHTOptionsMenu::OnGraphicsSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
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

void UHTOptionsMenu::OnResolutionSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UE_LOG(LogTemp, Warning, TEXT("Selected resolution: %s"), *SelectedItem);

	if (!ensure(GameUserSettings != nullptr))
	{
		return;
	}

	if (SelectedItem == "Custom ...")
	{
		return;
	}

	FString WidthString;
	FString HeightString;
	SelectedItem.Split("x", &WidthString, &HeightString);
	FIntPoint Resolution;
	Resolution.X = FCString::Atoi(*WidthString);
	Resolution.Y = FCString::Atoi(*HeightString);
	
	GameUserSettings->SetScreenResolution(Resolution);
}

void UHTOptionsMenu::OnWindowModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectedItem == "Fullscreen")
	{
		GameUserSettings->SetFullscreenMode(EWindowMode::Fullscreen);
	}

	if (SelectedItem == "Windowed Fullscreen")
	{
		GameUserSettings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
	}

	if (SelectedItem == "Windowed")
	{
		GameUserSettings->SetFullscreenMode(EWindowMode::Windowed);
	}
}

void UHTOptionsMenu::OnSaveButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Save button clicked."));

	GameUserSettings->ApplySettings(false);
}

void UHTOptionsMenu::SetupGraphicsSelection()
{
	if (GameUserSettings == nullptr)
	{
		return;
	}

	GraphicsSelection->AddOption(TEXT("Low"));
	GraphicsSelection->AddOption(TEXT("Medium"));
	GraphicsSelection->AddOption(TEXT("High"));
	GraphicsSelection->AddOption(TEXT("Epic"));
	GraphicsSelection->AddOption(TEXT("Custom"));

	int32 OverallScalabilityLevel = GameUserSettings->GetOverallScalabilityLevel();

	switch (OverallScalabilityLevel)
	{
	case 0:
		GraphicsSelection->SetSelectedIndex(0);
		break;
	case 1:
		GraphicsSelection->SetSelectedIndex(1);
		break;
	case 2:
		GraphicsSelection->SetSelectedIndex(2);
		break;
	case 3:
		GraphicsSelection->SetSelectedIndex(3);
		break;
	default:
		GraphicsSelection->SetSelectedIndex(4);
	}

	GraphicsSelection->OnSelectionChanged.AddDynamic(this, &UHTOptionsMenu::OnGraphicsSelectionChanged);
}

void UHTOptionsMenu::SetupResolutionSelection()
{
	if (GameUserSettings == nullptr)
	{
		return;
	}
	
	ResolutionSelection->AddOption(TEXT("1280x720"));
	ResolutionSelection->AddOption(TEXT("1600x900"));
	ResolutionSelection->AddOption(TEXT("1920x1080"));
	ResolutionSelection->AddOption(TEXT("2560x1440"));
	ResolutionSelection->AddOption(TEXT("3840x2160"));
	ResolutionSelection->AddOption(TEXT("Custom ..."));
	ResolutionSelection->SetSelectedIndex(5);

	const FIntPoint LastConfirmedScreenResolution = GameUserSettings->GetLastConfirmedScreenResolution();

	if (LastConfirmedScreenResolution == FIntPoint(1280, 720))
	{
		ResolutionSelection->SetSelectedIndex(0);
	}

	if (LastConfirmedScreenResolution == FIntPoint(1600, 900))
	{
		ResolutionSelection->SetSelectedIndex(1);
	}

	if (LastConfirmedScreenResolution == FIntPoint(1920, 1080))
	{
		ResolutionSelection->SetSelectedIndex(2);
	}

	if (LastConfirmedScreenResolution == FIntPoint(2560, 1440))
	{
		ResolutionSelection->SetSelectedIndex(3);
	}

	if (LastConfirmedScreenResolution == FIntPoint(3840, 2160))
	{
		ResolutionSelection->SetSelectedIndex(4);
	}
	
	ResolutionSelection->OnSelectionChanged.AddDynamic(this, &UHTOptionsMenu::OnResolutionSelectionChanged);
}

void UHTOptionsMenu::SetupWindowModeSelection()
{
	if (GameUserSettings == nullptr)
	{
		return;
	}

	WindowModeSelection->AddOption(TEXT("Fullscreen"));
	WindowModeSelection->AddOption(TEXT("Windowed Fullscreen"));
	WindowModeSelection->AddOption(TEXT("Windowed"));

	EWindowMode::Type FullscreenMode = GameUserSettings->GetFullscreenMode();

	switch (FullscreenMode)
	{
	case EWindowMode::Fullscreen:
		WindowModeSelection->SetSelectedIndex(0);
		break;
	case EWindowMode::WindowedFullscreen:
		WindowModeSelection->SetSelectedIndex(1);
		break;
	case EWindowMode::Windowed:
		WindowModeSelection->SetSelectedIndex(2);
		break;
	default:
		WindowModeSelection->SetSelectedIndex(2);
	}

	WindowModeSelection->OnSelectionChanged.AddDynamic(this, &UHTOptionsMenu::OnWindowModeSelectionChanged);
}

