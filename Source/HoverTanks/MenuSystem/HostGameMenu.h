// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HostGameMenu.generated.h"

class IMultiplayerGameControls;
class UButton;
class UComboBoxString;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHostGameMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual bool Initialize() override;

	void SetMultiplayerGameControls(IMultiplayerGameControls* InMultiplayerGameControls) { MultiplayerGameControls = InMultiplayerGameControls; }

private:
	IMultiplayerGameControls* MultiplayerGameControls;
	
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