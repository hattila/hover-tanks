// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OptionsMenu.generated.h"

class UButton;
class UComboBoxString;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UOptionsMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual bool Initialize() override;
	
private:

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* GraphicsSelection;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ResolutionSelection;

	UPROPERTY(meta = (BindWidget))
	UButton* SaveButton;

	bool IsEveryElementInitialized() const;
	
	UFUNCTION()
	void OnGraphicsSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	
	UFUNCTION()
	void OnResolutionSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSaveButtonClicked();
};
