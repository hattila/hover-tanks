// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HTOpenableMenu.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTOpenableMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual bool Initialize() override;

	virtual void Setup();
	virtual void Teardown();

	void SetupInputModeUIOnly();
	void SetupInputModeGameAndUi();
	void SetInputModeGameOnly() const;

protected:
	virtual bool IsEveryElementInitialized() const;
};
