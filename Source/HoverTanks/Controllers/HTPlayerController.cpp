// Fill out your copyright notice in the Description page of Project Settings.


#include "HTPlayerController.h"

#include "HoverTanks/Pawns/HoverTank/HTHoverTank.h"
#include "..\MenuSystem\HTInGameMenu.h"
#include "HoverTanks/Game/HTPSInTeam.h"
#include "HoverTanks/Game/GameStates/HTGSTeamDeathMatch.h"
#include "HoverTanks/Game/GameModes/HTGM_CanRequestRespawnInterface.h"
#include "HoverTanks/Game/GameModes/HTGM_HandlesTankDeathInterface.h"
#include "HoverTanks/UI/HUD/HT_ScoringHUDInterface.h"
#include "HoverTanks/UI/HUD/HTPlayerHUD.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"

AHTPlayerController::AHTPlayerController()
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

void AHTPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ClientSetInputModeToGameOnly();
	ClientCreatePlayerHUD();

	if (!HasAuthority())
	{
		return;
	}

	/**
	 * Listen to team changes on a team game.
	 */
	AHTPSInTeam* TeamPlayerState = GetPlayerState<AHTPSInTeam>();
	if (!TeamPlayerState)
	{
		FString RoleString;
		UEnum::GetValueAsString(GetLocalRole(), RoleString);

		UE_LOG(LogTemp, Warning, TEXT("AHTPlayerController::BeginPlay, role %s, no team player state"), *RoleString);
		return;
	}
	
	TeamPlayerState->OnTeamIdChanged.AddDynamic(this, &AHTPlayerController::ApplyTeamColorToPawn);
	TeamPlayerState->OnTeamIdChanged.AddDynamic(this, &AHTPlayerController::ServerRefreshMeOnTheScoreBoard);

	ApplyTeamColorToPawn(TeamPlayerState->GetTeamId());
	ServerRefreshMeOnTheScoreBoard(TeamPlayerState->GetTeamId());
}

void AHTPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (InGameMenu != nullptr)
	{
		InGameMenu->Teardown();
	}
}

void AHTPlayerController::ClientCreatePlayerHUD_Implementation()
{
	AHTPlayerHUD* HUD = Cast<AHTPlayerHUD>(GetHUD());
	if (HUD)
	{
		HUD->CreatePlayerHUD();
	}
}

void AHTPlayerController::ClientCreateTankHUD_Implementation(APawn* InPawn)
{
	AHTPlayerHUD* HUD = Cast<AHTPlayerHUD>(GetHUD());
	AHTHoverTank* HoverTank = Cast<AHTHoverTank>(InPawn);
	if (HUD && HoverTank)
	{
		HUD->CreateTankHUD(HoverTank);
	}
}

void AHTPlayerController::SetupInputComponent()
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
		EnhancedInputComponent->BindAction(OpenInGameMenuAction, ETriggerEvent::Started, this, &AHTPlayerController::OpenInGameMenuActionStarted);
		EnhancedInputComponent->BindAction(OpenScoreBoardAction, ETriggerEvent::Started, this, &AHTPlayerController::OpenScoreBoardActionStarted);
		EnhancedInputComponent->BindAction(RequestRespawnAction, ETriggerEvent::Started, this, &AHTPlayerController::RequestRespawnActionStarted);
	}
}

void AHTPlayerController::ClientOnScoresChanged_Implementation()
{
	IHT_ScoringHUDInterface* HUD = Cast<IHT_ScoringHUDInterface>(GetHUD());

	if (!ensure(HUD != nullptr))
	{
		return;
	}
	
	HUD->RefreshPlayerScores();
}

void AHTPlayerController::ClientForceOpenScoreBoard_Implementation(int32 TimeUntilRestartInSeconds)
{
	IHT_ScoringHUDInterface* HUD = Cast<IHT_ScoringHUDInterface>(GetHUD());

	if (!ensure(HUD != nullptr))
	{
		return;
	}

	HUD->ForceOpenScoreBoard();
}

void AHTPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	// log out the role
	// UE_LOG(LogTemp, Warning, TEXT("AHTPlayerController::OnRep_Pawn, role: %s"), *UEnum::GetValueAsString(GetLocalRole()));
}

void AHTPlayerController::ServerAttemptToJoinTeam_Implementation(int8 TeamId)
{
	AHTGSTeamDeathMatch* GameState = GetWorld()->GetGameState<AHTGSTeamDeathMatch>();
	if (GameState)
	{
		AHTPSInTeam* TeamPlayerState = GetPlayerState<AHTPSInTeam>();
		if (TeamPlayerState == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("AHTPlayerController::ServerAttemptToJoinTeam: TeamPlayerState is null"));
			return;
		}

		if (GetPawn() != nullptr)
		{
			AHTHoverTank* PossessedHoverTank = Cast<AHTHoverTank>(GetPawn());
			if (PossessedHoverTank && !PossessedHoverTank->IsDead())
			{
				PossessedHoverTank->ServerSuicide();
			}
		}
		
		GameState->AssignPlayerToTeam(TeamPlayerState, TeamId);
	}
}

void AHTPlayerController::ClientAddKillIndicator_Implementation(const FString& KillerName, const FString& VictimName, FLinearColor KillerColor, FLinearColor VictimColor)
{
	// log out the colors
	// UE_LOG(LogTemp, Warning, TEXT("KillerColor: %s, VictimColor: %s"), *KillerColor.ToString(), *VictimColor.ToString());
	
	// get the hud
	IHT_ScoringHUDInterface* HUD = Cast<IHT_ScoringHUDInterface>(GetHUD());
	if (HUD)
	{
		HUD->AddKillIndicator(KillerName, VictimName, KillerColor, VictimColor);
	}
}


void AHTPlayerController::ClientSetInputModeToGameOnly_Implementation()
{
	const FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);
	bShowMouseCursor = false;
}

/**
 * If there is a possessed pawn that has team colors, and it is a team game, apply the team color to the pawn.
 */
void AHTPlayerController::ApplyTeamColorToPawn(const int8 NewTeamId)
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
	const AHTGSTeamDeathMatch* GameState = GetWorld()->GetGameState<AHTGSTeamDeathMatch>();
	if (GameState == nullptr)
	{
		return;
	}

	TeamColorPawn->ApplyTeamColors(GameState->GetTeamDataAsset(NewTeamId));
}

void AHTPlayerController::ServerRefreshMeOnTheScoreBoard_Implementation(int8 NewTeamId)
{
	AHTGSTeamDeathMatch* GameState = GetWorld()->GetGameState<AHTGSTeamDeathMatch>();
	if (GameState == nullptr)
	{
		return;
	}

	GameState->InitializeNewPlayerScore(this);
}

void AHTPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (GetLocalRole() == ROLE_Authority)
	{
		
	}
}

void AHTPlayerController::OnUnPossess()
{
	if (GetLocalRole() == ROLE_Authority)
	{

	}
	
	Super::OnUnPossess();
}

void AHTPlayerController::OpenInGameMenuActionStarted()
{
	if (!ensure(InGameMenuClass != nullptr))
	{
		return;
	}

	if (InGameMenu == nullptr)
	{
		InGameMenu = CreateWidget<UHTInGameMenu>(this, InGameMenuClass);
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

void AHTPlayerController::OpenScoreBoardActionStarted()
{
	IHT_ScoringHUDInterface* HUD = Cast<IHT_ScoringHUDInterface>(GetHUD());

	if (!ensure(HUD != nullptr))
	{
		return;
	}
	
	HUD->ToggleScoreBoard();
}

void AHTPlayerController::RequestRespawnActionStarted()
{
	if (GetPawn() == nullptr)
	{
		ServerRequestRespawn();
		return;
	}
	
	AHTHoverTank* PossessedHoverTank = Cast<AHTHoverTank>(GetPawn());
	if (PossessedHoverTank && PossessedHoverTank->IsDead())
	{
		ServerRequestRespawn();
	}
	
}

void AHTPlayerController::ServerRequestRespawn_Implementation()
{
	IHTGM_CanRequestRespawnInterface* GameModeInterface =  Cast<IHTGM_CanRequestRespawnInterface>(GetWorld()->GetAuthGameMode());

	if (GameModeInterface == nullptr)
	{
		return;
	}
	
	if (GetPawn() == nullptr)
	{
		GameModeInterface->RequestRespawn(this);
		return;
	}
	
	AHTHoverTank* PossessedHoverTank = Cast<AHTHoverTank>(GetPawn());
	if (PossessedHoverTank && PossessedHoverTank->IsDead())
	{
		GameModeInterface->RequestRespawn(this);
	}
}
