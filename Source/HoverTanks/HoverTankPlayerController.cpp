// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankPlayerController.h"

#include "HoverTank.h"
#include "HoverTanks/MenuSystem/InGameMenu.h"
#include "HoverTanks/Components/HealthComponent.h"
#include "HoverTanks/Game/InTeamPlayerState.h"
#include "HoverTanks/Game/GameModes/TeamDeathMatchGameState.h"
#include "HoverTanks/Game/GameModes/DeathMatchGameMode.h"
#include "HoverTanks/UI/HUD/ScoringHUDInterface.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"

AHoverTankPlayerController::AHoverTankPlayerController()
{
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

	static ConstructorHelpers::FObjectFinder<UInputAction> FindInputActionOpenScoreBoardAction(TEXT("/Game/HoverTanks/Input/Actions/IA_OpenScoreBoard"));
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

void AHoverTankPlayerController::BeginPlay()
{
	Super::BeginPlay();

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

	if (InGameMenu != nullptr)
	{
		InGameMenu->Teardown();
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
		EnhancedInputComponent->BindAction(OpenInGameMenuAction, ETriggerEvent::Started, this, &AHoverTankPlayerController::OpenInGameMenuActionStarted);
		EnhancedInputComponent->BindAction(OpenScoreBoardAction, ETriggerEvent::Started, this, &AHoverTankPlayerController::OpenScoreBoardActionStarted);
		EnhancedInputComponent->BindAction(RequestRespawnAction, ETriggerEvent::Started, this, &AHoverTankPlayerController::RequestRespawnActionStarted);
	}
}

void AHoverTankPlayerController::ClientOnScoresChanged_Implementation()
{
	IScoringHUDInterface* HUD = Cast<IScoringHUDInterface>(GetHUD());

	if (!ensure(HUD != nullptr))
	{
		return;
	}
	
	HUD->RefreshPlayerScores();
}

void AHoverTankPlayerController::ClientForceOpenScoreBoard_Implementation(int32 TimeUntilRestartInSeconds)
{
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
				PossessedHoverTank->GetHealthComponent()->OnAnyDamage(PossessedHoverTank, 9999.f, nullptr, this, nullptr);
			}
		}
		
		GameState->AssignPlayerToTeam(TeamPlayerState, TeamId);
	}
}

/**
 * If there is a possessed pawn that has team colors, and it is a team game, apply the team color to the pawn.
 */
void AHoverTankPlayerController::ApplyTeamColorToPawn(const int8 NewTeamId)
{
	if (GetPawn() == nullptr)
	{
		return;
	}

	IHasTeamColors* TeamColorPawn = Cast<IHasTeamColors>(GetPawn());
	if (TeamColorPawn == nullptr)
	{
		return;
	}

	// get the teams data asset todo: TeamGameStateInterface
	const ATeamDeathMatchGameState* GameState = GetWorld()->GetGameState<ATeamDeathMatchGameState>();
	if (GameState == nullptr)
	{
		return;
	}

	TeamColorPawn->ApplyTeamColors(GameState->GetTeamDataAsset(NewTeamId));
}

void AHoverTankPlayerController::ServerRefreshMeOnTheScoreBoard_Implementation(int8 NewTeamId)
{
	ATeamDeathMatchGameState* GameState = GetWorld()->GetGameState<ATeamDeathMatchGameState>();
	if (GameState == nullptr)
	{
		return;
	}

	GameState->InitializeNewPlayerScore(this);
}

void AHoverTankPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (GetLocalRole() == ROLE_Authority)
	{

	}
}

void AHoverTankPlayerController::OnUnPossess()
{
	if (GetLocalRole() == ROLE_Authority)
	{

	}
	
	Super::OnUnPossess();
}

void AHoverTankPlayerController::OpenInGameMenuActionStarted()
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

void AHoverTankPlayerController::OpenScoreBoardActionStarted()
{
	IScoringHUDInterface* HUD = Cast<IScoringHUDInterface>(GetHUD());

	if (!ensure(HUD != nullptr))
	{
		return;
	}
	
	HUD->ToggleScoreBoard();
}

void AHoverTankPlayerController::RequestRespawnActionStarted()
{
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
	ICanRequestRespawnGameModeInterface* GameModeInterface =  Cast<ICanRequestRespawnGameModeInterface>(GetWorld()->GetAuthGameMode());

	if (GameModeInterface == nullptr)
	{
		return;
	}
	
	if (GetPawn() == nullptr)
	{
		GameModeInterface->RequestRespawn(this);
		return;
	}
	
	AHoverTank* PossessedHoverTank = Cast<AHoverTank>(GetPawn());
	if (PossessedHoverTank && PossessedHoverTank->IsDead())
	{
		GameModeInterface->RequestRespawn(this);
	}
}
