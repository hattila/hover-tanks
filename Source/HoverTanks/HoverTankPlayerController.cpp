// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankPlayerController.h"

#include "HoverTank.h"
#include "Game/DeathMatchGameState.h"
#include "MenuSystem/InGameMenu.h"
#include "UI/DeathMatchScoreBoardWidget.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Game/DeathMatchGameMode.h"
#include "Net/UnrealNetwork.h"

AHoverTankPlayerController::AHoverTankPlayerController():
	InGameMenu(nullptr),
	DeathMatchScoreBoardWidget(nullptr)
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
		TEXT("/Game/HoverTanks/UI/WBP_DeathMatchScoreBoardWidget"));
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

	static ConstructorHelpers::FObjectFinder<UInputAction> FindInputActionShoot(TEXT("/Game/HoverTanks/Input/Actions/IA_Jump"));
	if (FindInputActionShoot.Succeeded())
	{
		RequestRespawnAction = FindInputActionShoot.Object;
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
	if (DeathMatchScoreBoardWidget != nullptr && DeathMatchScoreBoardWidget->IsOpen())
	{
		DeathMatchScoreBoardWidget->RefreshPlayerScores(PlayerScores);
	}
}

void AHoverTankPlayerController::ClientForceOpenScoreBoard_Implementation()
{
	if (!ensure(DeathMatchScoreBoardClass != nullptr))
	{
		return;
	}

	if (DeathMatchScoreBoardWidget == nullptr)
	{
		DeathMatchScoreBoardWidget = CreateWidget<UDeathMatchScoreBoardWidget>(this, DeathMatchScoreBoardClass);
	}
	
	if (!ensure(DeathMatchScoreBoardWidget != nullptr))
	{
		return;
	}

	if (DeathMatchScoreBoardWidget->IsOpen())
	{
		return;
	}

	ADeathMatchGameState* DeathMatchGameState = GetWorld()->GetGameState<ADeathMatchGameState>();
	if (DeathMatchGameState)
	{
		DeathMatchScoreBoardWidget->SetTimeLeft(DeathMatchGameState->GetTimeRemaining());
	}
	DeathMatchScoreBoardWidget->Setup();
	DeathMatchScoreBoardWidget->RefreshPlayerScores(PlayerScores);
}

void AHoverTankPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// this->OnPossess.AddDynamic(this, &AHoverTankPlayerController::OnPossess);
	
	// todo: respawn able game mode interface?
	GameModeRef = Cast<ADeathMatchGameMode>(GetWorld()->GetAuthGameMode());
}

void AHoverTankPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// remove all widgets
	if (InGameMenu)
	{
		InGameMenu->Teardown();
	}

	if (DeathMatchScoreBoardWidget)
	{
		DeathMatchScoreBoardWidget->Teardown();
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

	if (DeathMatchScoreBoardWidget == nullptr)
	{
		DeathMatchScoreBoardWidget = CreateWidget<UDeathMatchScoreBoardWidget>(this, DeathMatchScoreBoardClass);
	}
	
	if (!ensure(DeathMatchScoreBoardWidget != nullptr))
	{
		return;
	}

	if (DeathMatchScoreBoardWidget->IsOpen())
	{
		DeathMatchScoreBoardWidget->Teardown();
		return;
	}

	// UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::OpenScoreBoard. Score count: %d"), PlayerScores.Num());

	ADeathMatchGameState* DeathMatchGameState = GetWorld()->GetGameState<ADeathMatchGameState>();
	if (DeathMatchGameState)
	{
		DeathMatchScoreBoardWidget->SetTimeLeft(DeathMatchGameState->GetTimeRemaining());
	}
	DeathMatchScoreBoardWidget->Setup();
	DeathMatchScoreBoardWidget->RefreshPlayerScores(PlayerScores);
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
		// request respawn
		GameModeRef->RequestRespawn(this);
	}	
}

void AHoverTankPlayerController::OnRep_PlayerScores() const
{
	if (DeathMatchScoreBoardWidget)
	{
		DeathMatchScoreBoardWidget->RefreshPlayerScores(PlayerScores);
	}
}
