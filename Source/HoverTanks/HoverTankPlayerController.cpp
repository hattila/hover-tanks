// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankPlayerController.h"

#include "HoverTank.h"
#include "MenuSystem/InGameMenu.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Game/DeathMatchGameMode.h"
#include "UI/HUD/DeathMatchHUD.h"

AHoverTankPlayerController::AHoverTankPlayerController():
	InGameMenu(nullptr),
	GameModeRef(nullptr)
{
	// initialize InGameMenuClass
	static ConstructorHelpers::FClassFinder<UUserWidget> InGameMenuClassFinder(
		TEXT("/Game/HoverTanks/Menu/WBP_InGameMenu"));
	if (InGameMenuClassFinder.Succeeded())
	{
		InGameMenuClass = InGameMenuClassFinder.Class;
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

	static ConstructorHelpers::FObjectFinder<UInputAction> FindInputActionShoot(
		TEXT("/Game/HoverTanks/Input/Actions/IA_Jump"));
	if (FindInputActionShoot.Succeeded())
	{
		RequestRespawnAction = FindInputActionShoot.Object;
	}
}

void AHoverTankPlayerController::ClientOnScoresChanged_Implementation()
{
	ADeathMatchHUD* HUD = Cast<ADeathMatchHUD>(GetHUD());
	HUD->RefreshPlayerScores();
}

void AHoverTankPlayerController::ClientForceOpenScoreBoard_Implementation(int32 TimeUntilRestartInSeconds)
{
	ADeathMatchHUD* HUD = Cast<ADeathMatchHUD>(GetHUD());
	HUD->ForceOpenScoreBoard();
}

void AHoverTankPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// todo: respawn able game mode interface?
	GameModeRef = Cast<ADeathMatchGameMode>(GetWorld()->GetAuthGameMode());
}

void AHoverTankPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// remove all widgets
	if (InGameMenu != nullptr)
	{
		InGameMenu->Teardown(); // still can crash
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
		EnhancedInputComponent->BindAction(RequestRespawnAction, ETriggerEvent::Started, this, &AHoverTankPlayerController::RequestRespawn);
	}
}

void AHoverTankPlayerController::OnPossess(APawn* InPawn)
{
	// UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::OnPossess"));
	
	Super::OnPossess(InPawn);

	if (GetLocalRole() == ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("SERVER before client call, Pawn is %s"), InPawn != nullptr ? *InPawn->GetClass()->GetName() : TEXT("null"));
		
		FString InPawnClassName = InPawn->GetClass()->GetName();
		ClientAddHUDWidget(InPawnClassName);
	}
}

void AHoverTankPlayerController::OnUnPossess()
{
	// UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::OnUnPossess"));

	if (GetLocalRole() == ROLE_Authority)
	{
		ClientRemoveHUDWidget();
	}
	
	Super::OnUnPossess();
}

void AHoverTankPlayerController::ClientAddHUDWidget_Implementation(const FString& InPawnClassName)
{
	OnPawnPossessed.Broadcast(InPawnClassName);
}

void AHoverTankPlayerController::ClientRemoveHUDWidget_Implementation()
{
	const FString InPawnClassName = GetPawn()->GetClass()->GetName();
	OnPawnUnPossessed.Broadcast(InPawnClassName);
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
	ADeathMatchHUD* HUD = Cast<ADeathMatchHUD>(GetHUD());
	HUD->ToggleScoreBoard();
}

void AHoverTankPlayerController::RequestRespawn()
{
	AHoverTank* PossessedHoverTank = Cast<AHoverTank>(GetPawn());
	if (PossessedHoverTank && PossessedHoverTank->IsDead())
	{
		ServerRequestRespawn();
	}
}

void AHoverTankPlayerController::ServerRequestRespawn_Implementation()
{
	AHoverTank* PossessedHoverTank = Cast<AHoverTank>(GetPawn());
	if (PossessedHoverTank && PossessedHoverTank->IsDead() && GameModeRef != nullptr)
	{
		GameModeRef->RequestRespawn(this);
	}	
}
