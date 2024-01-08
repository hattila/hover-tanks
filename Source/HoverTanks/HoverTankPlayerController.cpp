// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankPlayerController.h"

#include "HoverTank.h"
#include "MenuSystem/InGameMenu.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Components/HealthComponent.h"
#include "Game/InTeamPlayerState.h"
#include "Game/GameModes/TeamDeathMatchGameState.h"
#include "HoverTanks/Game/GameModes/DeathMatchGameMode.h"
#include "HoverTanks/UI/HUD/ScoringHUDInterface.h"

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
	// AInGameHUD* HUD = Cast<AInGameHUD>(GetHUD());
	IScoringHUDInterface* HUD = Cast<IScoringHUDInterface>(GetHUD());

	if (!ensure(HUD != nullptr))
	{
		return;
	}
	
	HUD->RefreshPlayerScores();
}

void AHoverTankPlayerController::ClientForceOpenScoreBoard_Implementation(int32 TimeUntilRestartInSeconds)
{
	// AInGameHUD* HUD = Cast<AInGameHUD>(GetHUD());
	IScoringHUDInterface* HUD = Cast<IScoringHUDInterface>(GetHUD());

	if (!ensure(HUD != nullptr))
	{
		return;
	}

	HUD->ForceOpenScoreBoard();
}

void AHoverTankPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
}

void AHoverTankPlayerController::ServerAttemptToJoinTeam_Implementation(int8 TeamId)
{
	ATeamDeathMatchGameState* GameState = GetWorld()->GetGameState<ATeamDeathMatchGameState>();
	if (GameState)
	{
		// get player state
		AInTeamPlayerState* TeamPlayerState = GetPlayerState<AInTeamPlayerState>();
		if (TeamPlayerState == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::ServerAttemptToJoinTeam: TeamPlayerState is null"));
			return;
		}

		if (GetPawn() != nullptr)
		{
			AHoverTank* PossessedHoverTank = Cast<AHoverTank>(GetPawn());
			if (PossessedHoverTank && !PossessedHoverTank->IsDead())
			{
				PossessedHoverTank->GetHealthComponent()->OnAnyDamage(PossessedHoverTank, 999.f, nullptr, this, nullptr);
			}
		}
		
		GameState->AssignPlayerToTeam(TeamPlayerState, TeamId);
	}
}

void AHoverTankPlayerController::ApplyTeamColorToPawn(int8 NewTeamId)
{
	// FString RoleString;
	// UEnum::GetValueAsString(GetLocalRole(), RoleString);
	// UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::ApplyTeamColorToPawn, role %s, team id: %d"), *RoleString, NewTeamId);

	// do we have a possessed pawn?
	if (GetPawn() == nullptr)
	{
		return;
	}

	// pawn has team colors?
	IHasTeamColors* TeamColorPawn = Cast<IHasTeamColors>(GetPawn());
	if (TeamColorPawn == nullptr)
	{
		return;
	}

	// get the teams data asset todo: TeamGameStateInterface
	ATeamDeathMatchGameState* GameState = GetWorld()->GetGameState<ATeamDeathMatchGameState>();
	if (GameState == nullptr)
	{
		return;
	}

	TeamColorPawn->ApplyTeamColors(GameState->GetTeamDataAsset(NewTeamId));
}

void AHoverTankPlayerController::ServerRefreshMeOnTheScoreBoard_Implementation(int8 NewTeamId)
{
	// FString RoleString;
	// UEnum::GetValueAsString(GetLocalRole(), RoleString);
	// UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::ServerRefreshMeOnTheScoreBoard_Implementation, role %s, team id: %d"), *RoleString, NewTeamId);
	
	ATeamDeathMatchGameState* GameState = GetWorld()->GetGameState<ATeamDeathMatchGameState>();
	if (GameState == nullptr)
	{
		return;
	}

	GameState->InitializeNewPlayerScore(this);
}

void AHoverTankPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// todo: respawn able game mode interface?
	GameModeRef = Cast<ADeathMatchGameMode>(GetWorld()->GetAuthGameMode());

	FString RoleString;
	UEnum::GetValueAsString(GetLocalRole(), RoleString);

	if (!HasAuthority())
	{
		return;
	}

	/**
	 * Listen to team changes on a team game.
	 */
	AInTeamPlayerState* TeamPlayerState = GetPlayerState<AInTeamPlayerState>();
	if (!TeamPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::BeginPlay, role %s, no team player state"), *RoleString);
		return;
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::BeginPlay, role %s, team id: %d"), *RoleString, TeamPlayerState->GetTeamId());
	
	TeamPlayerState->OnTeamIdChanged.AddDynamic(this, &AHoverTankPlayerController::ApplyTeamColorToPawn);
	TeamPlayerState->OnTeamIdChanged.AddDynamic(this, &AHoverTankPlayerController::ServerRefreshMeOnTheScoreBoard);
	ApplyTeamColorToPawn(TeamPlayerState->GetTeamId());
	ServerRefreshMeOnTheScoreBoard(TeamPlayerState->GetTeamId());
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
	IScoringHUDInterface* HUD = Cast<IScoringHUDInterface>(GetHUD());

	if (!ensure(HUD != nullptr))
	{
		return;
	}
	
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
