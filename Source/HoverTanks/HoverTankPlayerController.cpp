// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankPlayerController.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "MenuSystem/InGameMenu.h"
#include "Net/UnrealNetwork.h"
#include "UI/DeathMatchScoreBoard.h"

AHoverTankPlayerController::AHoverTankPlayerController():
	InGameMenu(nullptr),
	DeathMatchScoreBoard(nullptr)
{
	// initialize InGameMenuClass
	static ConstructorHelpers::FClassFinder<UUserWidget> InGameMenuClassFinder(
		TEXT("/Game/HoverTanks/Menu/WBP_InGameMenu"));
	if (InGameMenuClassFinder.Succeeded())
	{
		InGameMenuClass = InGameMenuClassFinder.Class;
	}

	// initialize DeathMatchScoreBoardClass
	static ConstructorHelpers::FClassFinder<UUserWidget> DeathMatchScoreBoardClassFinder(
		TEXT("/Game/HoverTanks/UI/WBP_DeathMatchScoreBoard"));
	if (DeathMatchScoreBoardClassFinder.Succeeded())
	{
		DeathMatchScoreBoardClass = DeathMatchScoreBoardClassFinder.Class;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> FindInputMappingContext(
		TEXT("/Game/HoverTanks/Input/IMC_HoverTankPlayerController"));
	if (FindInputMappingContext.Succeeded())
	{
		HoverTankPlayerControllerInputContext = FindInputMappingContext.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> FindInputActionOpenInGameMenu(
		TEXT("/Game/HoverTanks/Input/Actions/IA_InGameMenu"));
	if (FindInputActionOpenInGameMenu.Succeeded())
	{
		OpenInGameMenuAction = FindInputActionOpenInGameMenu.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> FindInputActionOpenScoreBoardAction(
		TEXT("/Game/HoverTanks/Input/Actions/IA_OpenScoreBoard"));
	if (FindInputActionOpenScoreBoardAction.Succeeded())
	{
		OpenScoreBoardAction = FindInputActionOpenScoreBoardAction.Object;
	}
}

void AHoverTankPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHoverTankPlayerController, PlayerScores);
}

void AHoverTankPlayerController::ServerOnScoresChanged_Implementation(const TArray<FDeathMatchPlayerScore>& InPlayerScores)
{
	PlayerScores = InPlayerScores;
		
	// refresh a might be open scoreboard for the server player
	if (DeathMatchScoreBoard)
	{
		DeathMatchScoreBoard->RefreshPlayerScores(PlayerScores);
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
		EnhancedInputComponent->BindAction(OpenScoreBoardAction, ETriggerEvent::Started, this, &AHoverTankPlayerController::OpenScoreBoard);
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

void AHoverTankPlayerController::OpenScoreBoard()
{
	if (!ensure(DeathMatchScoreBoardClass != nullptr))
	{
		return;
	}

	if (DeathMatchScoreBoard == nullptr)
	{
		DeathMatchScoreBoard = CreateWidget<UDeathMatchScoreBoard>(this, DeathMatchScoreBoardClass);
	}
	
	if (!ensure(DeathMatchScoreBoard != nullptr))
	{
		return;
	}

	if (DeathMatchScoreBoard->IsOpen())
	{
		DeathMatchScoreBoard->Teardown();
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::OpenScoreBoard. Score count: %d"), PlayerScores.Num());
	
	DeathMatchScoreBoard->Setup();
	DeathMatchScoreBoard->RefreshPlayerScores(PlayerScores);
}

void AHoverTankPlayerController::OnRep_PlayerScores() const
{
	if (DeathMatchScoreBoard)
	{
		DeathMatchScoreBoard->RefreshPlayerScores(PlayerScores);
	}
}
