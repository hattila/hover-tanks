// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankPlayerController.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "MenuSystem/InGameMenu.h"

AHoverTankPlayerController::AHoverTankPlayerController(): InGameMenu(nullptr)
{
	// initialize InGameMenuClass
	static ConstructorHelpers::FClassFinder<UUserWidget> InGameMenuClassFinder(TEXT("/Game/HoverTanks/Menu/WBP_InGameMenu"));
	if (InGameMenuClassFinder.Succeeded())
	{
		InGameMenuClass = InGameMenuClassFinder.Class;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> FindInputMappingContext(TEXT("/Game/HoverTanks/Input/IMC_HoverTankPlayerController"));
	if (FindInputMappingContext.Succeeded())
	{
		HoverTankPlayerControllerInputContext = FindInputMappingContext.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> FindInputActionOpenInGameMenu(TEXT("/Game/HoverTanks/Input/Actions/IA_InGameMenu"));
	if (FindInputActionOpenInGameMenu.Succeeded())
	{
		OpenInGameMenuAction = FindInputActionOpenInGameMenu.Object;
	}
}

void AHoverTankPlayerController::OnScoresChanged(TArray<FDeathMatchPlayerScore> PlayerScores)
{
	UE_LOG(LogTemp, Warning, TEXT("Scores changed, refreshing scoreboard on PlayerController"));

	if (GEngine)
	{
		// iterate over player scores and create a multiline string
		FString PlayerScoresString = "";
		for (FDeathMatchPlayerScore PlayerScore : PlayerScores)
		{
			PlayerScoresString += FString::Printf(TEXT("%s: %d\n"), *PlayerScore.PlayerName, PlayerScore.Score);
		}
		
		FString Message = FString::Printf(TEXT("Scores changed, refreshing scoreboard on PlayerController. Scores\n %s"), *PlayerScoresString);
		GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Green, *Message);	
	}
}

void AHoverTankPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputLocalPlayerSubsystem* EnhancedInputLocalPlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (EnhancedInputLocalPlayerSubsystem)
	{
		EnhancedInputLocalPlayerSubsystem->AddMappingContext(HoverTankPlayerControllerInputContext, 0);
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(OpenInGameMenuAction, ETriggerEvent::Started, this, &AHoverTankPlayerController::OpenInGameMenu);
	}
}

void AHoverTankPlayerController::OpenInGameMenu()
{
	if (!ensure(InGameMenuClass != nullptr))
	{
		return;
	}

	if (InGameMenu == nullptr)
	{
		InGameMenu = CreateWidget<UInGameMenu>(this, InGameMenuClass);
	}
	
	if (!ensure(InGameMenu != nullptr))
	{
		return;
	}

	if (InGameMenu->IsOpen())
	{
		InGameMenu->Teardown();
		return;
	}

	InGameMenu->Setup();
}
