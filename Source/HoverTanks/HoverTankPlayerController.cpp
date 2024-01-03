// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankPlayerController.h"

#include "HoverTank.h"
#include "MenuSystem/InGameMenu.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Components/HoverTankEffectsComponent.h"
#include "Game/InTeamPlayerState.h"
#include "Game/GameModes/TeamDeathMatchGameState.h"
#include "Game/Teams/Team.h"
#include "HoverTanks/Game/GameModes/DeathMatchGameMode.h"
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

void AHoverTankPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
}

void AHoverTankPlayerController::ApplyTeamColorToPawn(int8 NewTeamId)
{
	// log
	FString RoleString;
	UEnum::GetValueAsString(GetLocalRole(), RoleString);
	UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::OnTeamIdChanged, role %s, team id: %d"), *RoleString, NewTeamId);

	// do we have a possessed pawn?
	if (GetPawn() == nullptr)
	{
		return;
	}

	// pawn has team id?
	IHasTeamColors* TeamColorPawn = Cast<IHasTeamColors>(GetPawn());
	if (TeamColorPawn == nullptr)
	{
		return;
	}

	// get the teams data asset
	ATeamDeathMatchGameState* GameState = GetWorld()->GetGameState<ATeamDeathMatchGameState>();
	if (GameState == nullptr)
	{
		return;
	}
	
	TeamColorPawn->ApplyTeamColors(GameState->GetTeamDataAsset(NewTeamId));
}

void AHoverTankPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// todo: respawn able game mode interface?
	GameModeRef = Cast<ADeathMatchGameMode>(GetWorld()->GetAuthGameMode());

	// is it a team game?
	// get PlayerState
	AInTeamPlayerState* TeamPlayerState = GetPlayerState<AInTeamPlayerState>();
	if (TeamPlayerState)
	{
		// log
		FString RoleString;
		UEnum::GetValueAsString(GetLocalRole(), RoleString);
		UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::BeginPlay, role %s, team id: %d"), *RoleString, TeamPlayerState->GetTeamId());

		TeamPlayerState->OnTeamIdChanged.AddDynamic(this, &AHoverTankPlayerController::ApplyTeamColorToPawn);
	}
	
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
		FString RoleString;
		UEnum::GetValueAsString(GetLocalRole(), RoleString);
		
		// get the player state
		AInTeamPlayerState* TeamPlayerState = GetPlayerState<AInTeamPlayerState>();
		if (TeamPlayerState)
		{
			UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::OnPossess, role %s, team id: %d, subscribing to onTeamChange"), *RoleString, TeamPlayerState->GetTeamId());
			ApplyTeamColorToPawn(TeamPlayerState->GetTeamId());
			
			AHoverTank* HoverTank = Cast<AHoverTank>(InPawn);
			TeamPlayerState->OnTeamIdChanged.AddDynamic(HoverTank->GetEffectsComponent(), &UHoverTankEffectsComponent::OnTeamIdChanged);
			// TeamPlayerState->OnTeamIdChanged.AddDynamic(HoverTank, &AHoverTank::ApplyTeamColors);

	
			
			// TeamPlayerState->OnTeamIdChanged.AddDynamic(this, &AHoverTankPlayerController::OnTeamIdChangeHandler);
			// HoverTank->GetEffectsComponent()->OnTeamIdChanged(TeamPlayerState->GetTeamId());
		}
	}
}

void AHoverTankPlayerController::OnUnPossess()
{
	// UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::OnUnPossess"));

	if (GetLocalRole() == ROLE_Authority)
	{

	}
	
	Super::OnUnPossess();
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
	// UE_LOG(LogTemp, Warning, TEXT("Owning a Pawn: %s"), GetPawn() ? *GetPawn()->GetName() : TEXT("null"));

	// if Controller does not possess a Pawn, then RequestRespawn
	if (GetPawn() == nullptr)
	{
		ServerRequestRespawn();
		return;
	}
	
	AHoverTank* PossessedHoverTank = Cast<AHoverTank>(GetPawn());
	if (PossessedHoverTank && PossessedHoverTank->IsDead())
	{
		ServerRequestRespawn();
	}
	
}

void AHoverTankPlayerController::ServerRequestRespawn_Implementation()
{
	if (GetPawn() == nullptr)
	{
		GameModeRef->RequestRespawn(this);
		return;
	}
	
	AHoverTank* PossessedHoverTank = Cast<AHoverTank>(GetPawn());
	if (PossessedHoverTank && PossessedHoverTank->IsDead() && GameModeRef != nullptr)
	{
		GameModeRef->RequestRespawn(this);
	}
}
