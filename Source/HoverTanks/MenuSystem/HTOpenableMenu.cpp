// Fill out your copyright notice in the Description page of Project Settings.


#include "HTOpenableMenu.h"

bool UHTOpenableMenu::Initialize()
{
	return Super::Initialize();
}

void UHTOpenableMenu::Setup()
{
	AddToViewport();
	SetupInputModeUIOnly();
}

void UHTOpenableMenu::Teardown()
{
	RemoveFromParent();
	SetInputModeGameOnly();
}

void UHTOpenableMenu::SetupInputModeUIOnly()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	
	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(true);
}

void UHTOpenableMenu::SetupInputModeGameAndUi()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	FInputModeGameAndUI InputModeData;
	InputModeData.SetWidgetToFocus(TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	
	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(true);
}

void UHTOpenableMenu::SetInputModeGameOnly() const
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	const FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);

	PlayerController->bShowMouseCursor = false;
}

bool UHTOpenableMenu::IsEveryElementInitialized() const
{
	return true;
}