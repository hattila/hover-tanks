// Fill out your copyright notice in the Description page of Project Settings.

#include "DeathMatchHUD.h"

#include "DeathMatchPlayerHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "HoverTanks/Game/DeathMatchGameState.h"

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

	DeathMatchPlayerHUDWidget = CreateWidget<UDeathMatchPlayerHUDWidget>(GetWorld(), DeathMatchPlayerHUDWidgetClass);
	if (APlayerController* PlayerController = GetOwningPlayerController())
	{
		DeathMatchPlayerHUDWidget->Setup();
	}

	ADeathMatchGameState* DeathMatchGameState = GetWorld()->GetGameState<ADeathMatchGameState>();
	if (DeathMatchGameState && DeathMatchPlayerHUDWidget)
	{
		DeathMatchPlayerHUDWidget->SetTimeLeft(DeathMatchGameState->GetTimeRemaining());
	}
}

void ADeathMatchHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (DeathMatchPlayerHUDWidget)
	{
		DeathMatchPlayerHUDWidget->Teardown();
	}
}


