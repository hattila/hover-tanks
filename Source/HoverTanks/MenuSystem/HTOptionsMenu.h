// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HTOptionsMenu.generated.h"

class UButton;
class UComboBoxString;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTOptionsMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual bool Initialize() override;
	
private:
	UPROPERTY()
	UGameUserSettings* GameUserSettings;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* GraphicsSelection;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ResolutionSelection;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* WindowModeSelection;

	UPROPERTY(meta = (BindWidget))
	UButton* SaveButton;

	bool IsEveryElementInitialized() const;
	
	UFUNCTION()
	void OnGraphicsSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	
	UFUNCTION()
	void OnResolutionSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnWindowModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSaveButtonClicked();

	void SetupGraphicsSelection();
	void SetupResolutionSelection();
	void SetupWindowModeSelection();
};
