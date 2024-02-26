// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankPlayerController.h"

#include "HoverTanks/Pawns/HoverTank.h"
#include "HoverTanks/MenuSystem/InGameMenu.h"
#include "HoverTanks/Components/HealthComponent.h"
#include "HoverTanks/Game/InTeamPlayerState.h"
#include "HoverTanks/Game/GameStates/TeamDeathMatchGameState.h"
#include "HoverTanks/Game/GameModes/CanRequestRespawnGameModeInterface.h"
#include "HoverTanks/UI/HUD/ScoringHUDInterface.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "HoverTanks/GAS/HTAttributeSetBase.h"
#include "HoverTanks/UI/HUD/HTPlayerHUD.h"

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

	ClientSetInputModeToGameOnly();
	ClientCreatePlayerHUD();

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
		FString RoleString;
		UEnum::GetValueAsString(GetLocalRole(), RoleString);

		UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::BeginPlay, role %s, no team player state"), *RoleString);
		return;
	}
	
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

void AHoverTankPlayerController::ClientCreatePlayerHUD_Implementation()
{
	AHTPlayerHUD* HUD = Cast<AHTPlayerHUD>(GetHUD());
	if (HUD)
	{
		HUD->CreatePlayerHUD();
	}
}

void AHoverTankPlayerController::ClientCreateTankHUD_Implementation(APawn* InPawn)
{
	AHTPlayerHUD* HUD = Cast<AHTPlayerHUD>(GetHUD());
	AHoverTank* HoverTank = Cast<AHoverTank>(InPawn);
	if (HUD && HoverTank)
	{
		HUD->CreateTankHUD(HoverTank);
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

	// log out the role
	// UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::OnRep_Pawn, role: %s"), *UEnum::GetValueAsString(GetLocalRole()));
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
				PossessedHoverTank->ServerSuicide();
			}
		}
		
		GameState->AssignPlayerToTeam(TeamPlayerState, TeamId);
	}
}

void AHoverTankPlayerController::ClientAddKillIndicator_Implementation(const FString& KillerName, const FString& VictimName, FLinearColor KillerColor, FLinearColor VictimColor)
{
	// log out the colors
	// UE_LOG(LogTemp, Warning, TEXT("KillerColor: %s, VictimColor: %s"), *KillerColor.ToString(), *VictimColor.ToString());
	
	// get the hud
	IScoringHUDInterface* HUD = Cast<IScoringHUDInterface>(GetHUD());
	if (HUD)
	{
		HUD->AddKillIndicator(KillerName, VictimName, KillerColor, VictimColor);
	}
}


void AHoverTankPlayerController::ClientSetInputModeToGameOnly_Implementation()
{
	const FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);
	bShowMouseCursor = false;
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

	// log
	// UE_LOG(LogTemp, Warning, TEXT("AHoverTankPlayerController::OnPossess, role: %s, possessing a %s"), *UEnum::GetValueAsString(GetLocalRole()), *InPawn->GetName());
	// ClientCreateTankHUD(InPawn);
	
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
