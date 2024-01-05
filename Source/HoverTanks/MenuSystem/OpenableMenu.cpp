// Fill out your copyright notice in the Description page of Project Settings.


#include "OpenableMenu.h"

bool UOpenableMenu::Initialize()
{
	return Super::Initialize();
}

void UOpenableMenu::Setup()
{
	AddToViewport();
	SetupInputModeUIOnly();
}

void UOpenableMenu::Teardown()
{
	RemoveFromParent();
	SetInputModeGameOnly();
}

void UOpenableMenu::SetupInputModeUIOnly()
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

void UOpenableMenu::SetupInputModeGameAndUi()
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

void UOpenableMenu::SetInputModeGameOnly() const
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

bool UOpenableMenu::IsEveryElementInitialized()
{
	return true;
}