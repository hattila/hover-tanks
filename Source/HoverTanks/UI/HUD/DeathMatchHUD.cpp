// Fill out your copyright notice in the Description page of Project Settings.

#include "DeathMatchHUD.h"

#include "Blueprint/UserWidget.h"

ADeathMatchHUD::ADeathMatchHUD():
	DeathMatchPlayerHUDWidget(nullptr)
{
	// get hold of the blueprint versions of the widgets
	static ConstructorHelpers::FClassFinder<UUserWidget> DeathMatchPlayerHUDWidgetClassFinder(
		TEXT("/Game/HoverTanks/UI/HUD/WBP_DeathMatchPlayerHUDWidget"));
	if (DeathMatchPlayerHUDWidgetClassFinder.Succeeded())
	{
		DeathMatchPlayerHUDWidgetClass = DeathMatchPlayerHUDWidgetClassFinder.Class;
	}
}

void ADeathMatchHUD::BeginPlay()
{
	Super::BeginPlay();

	if (DeathMatchPlayerHUDWidgetClass == nullptr)
	{
		return;
	}

	DeathMatchPlayerHUDWidget = CreateWidget<UUserWidget>(GetWorld(), DeathMatchPlayerHUDWidgetClass);
	if (APlayerController* PlayerController = GetOwningPlayerController())
	{
		DeathMatchPlayerHUDWidget->AddToViewport();
	}
}


