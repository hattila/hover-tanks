// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HTHostGameMenu.generated.h"

class IHTGI_MultiplayerGameControlsInterface;
class UButton;
class UComboBoxString;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTHostGameMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual bool Initialize() override;

	void SetMultiplayerGameControls(IHTGI_MultiplayerGameControlsInterface* InMultiplayerGameControls) { MultiplayerGameControls = InMultiplayerGameControls; }

private:
	IHTGI_MultiplayerGameControlsInterface* MultiplayerGameControls;
	
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* MapSelection;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* GameModeSelection;

	UPROPERTY(meta = (BindWidget))
	UButton* StartGame;
	
	UFUNCTION()
	void OnMapSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnStartGameClicked();
};
