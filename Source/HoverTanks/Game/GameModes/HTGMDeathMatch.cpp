// Fill out your copyright notice in the Description page of Project Settings.


#include "HTGMDeathMatch.h"

#include "HoverTanks/Game/GameStates/HTGSDeathMatch.h"
#include "HoverTanks/Pawns/HoverTank/HTHoverTank.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "HoverTanks/Controllers/HTPlayerController.h"
#include "HoverTanks/Game/HTPlayerState.h"
#include "HoverTanks/UI/HUD/HTDeathMatchHUD.h"

#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AHTGMDeathMatch::AHTGMDeathMatch()
{
	/**
	 * Work started on Blueprint GameModes, where the default pawn could be set. Currently, the game uses the CPP class
	 * on OpenLevel calls to set the Game param to be the proper game mode
	 */
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/HoverTanks/Pawns/HoverTank/BP_HoverTank"));
	// static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/HoverTanks/Pawns/HoverTank/Big/BP_HoverTank_Big"));
	// static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/HoverTanks/Pawns/HoverTank/Small/BP_HoverTank_Small"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PlayerControllerClass = AHTPlayerController::StaticClass();

	TArray<AActor*> SpawnPointsInWorld;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPointsInWorld);
	for (AActor* SpawnPoint : SpawnPointsInWorld)
	{
		SpawnPoints.Add(Cast<APlayerStart>(SpawnPoint));
	}

	// Custom GameInstance should be set in DefaultEngine.ini

	PlayerStateClass = AHTPlayerState::StaticClass();
	GameStateClass = AHTGSDeathMatch::StaticClass();
	HUDClass = AHTDeathMatchHUD::StaticClass();

	bUseSeamlessTravel = false;
}

void AHTGMDeathMatch::TankDies(AHTHoverTank* DeadHoverTank, AController* DeathCauser)
{
	// UE_LOG(LogTemp, Warning, TEXT("Tank %s died!"), *DeadHoverTank->GetName());

	APlayerController* DeadPlayerController = Cast<APlayerController>(DeadHoverTank->GetController());
	if (DeadPlayerController)
	{
		SomeoneKilledSomeone(DeathCauser, DeadPlayerController);

		APlayerController* KillerPlayerController = Cast<APlayerController>(DeathCauser);
		
		AHTGSDeathMatch* DeathMatchGameState = GetGameState<AHTGSDeathMatch>();
		if (DeathMatchGameState && KillerPlayerController != nullptr)
		{
			DeathMatchGameState->AddScoreToPlayer(KillerPlayerController, 1);
		}

		DeadHoverTank->OnDeath();
	}
	
}

void AHTGMDeathMatch::RequestRespawn(APlayerController* InPlayerController)
{
	AHTGSDeathMatch* DeathMatchGameState = GetGameState<AHTGSDeathMatch>();

	if (DeathMatchGameState)
	{
		const int32 TimeRemaining = DeathMatchGameState->GetTimeRemaining();
		if (TimeRemaining > 0 && MatchState == EMatchState::InProgress)
		{
			APawn* CurrentPawn = InPlayerController->GetPawn();
			InPlayerController->UnPossess();
			if (CurrentPawn)
			{
				CurrentPawn->Destroy();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AHTDeathMatchGameMode::RequestRespawn - CurrentPawn is null"));
			}

			APlayerStart* RandomSpawnPoint = FindRandomSpawnPoint();
			AHTHoverTank* NewHoverTank = SpawnTankAtPlayerStart(RandomSpawnPoint);

			InPlayerController->Possess(NewHoverTank);
		}
	}
}

void AHTGMDeathMatch::BeginPlay()
{
	Super::BeginPlay();
	// UE_LOG(LogTemp, Warning, TEXT("DeathMatchGameMode BeginPlay %d"), MatchTimeInSeconds);

	AHTGSDeathMatch* DeathMatchGameState = GetGameState<AHTGSDeathMatch>();
	if (DeathMatchGameState)
	{
		DeathMatchGameState->SetTimeRemaining(MatchTimeInSeconds);
	}

	GetWorldTimerManager().SetTimer(GameTimerHandle, this, &AHTGMDeathMatch::OnOneSecondElapsed, 1.f, true);

	// recreate player scores
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (PlayerController && DeathMatchGameState)
		{
			DeathMatchGameState->InitializeNewPlayerScore(PlayerController);
		}
	}
}

void AHTGMDeathMatch::OnOneSecondElapsed()
{
	AHTGSDeathMatch* DeathMatchGameState = GetGameState<AHTGSDeathMatch>();
	if (DeathMatchGameState)
	{
		const int32 TimeRemaining = DeathMatchGameState->GetTimeRemaining();
		DeathMatchGameState->SetTimeRemaining(TimeRemaining - 1);

		if (TimeRemaining <= 0 && MatchState == EMatchState::InProgress)
		{
			GameOver();
		}
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("One second elapsed in %s state"), *UEnum::GetValueAsString(MatchState));
}

/**
 * GameOver is called when the match time runs out.
 *
 * Restarts the GameTimer, it now acts as a countdown to level restart.
 * Disable every players tank, force open the scoreboard for everyone.
 */
void AHTGMDeathMatch::GameOver()
{
	// clear the timer
	GetWorldTimerManager().ClearTimer(GameTimerHandle);
	MatchState = EMatchState::GameOver;
	const float GameRestartDelay = 10.f;

	// UE_LOG(LogTemp, Warning, TEXT("GameOver!"));

	AHTGSDeathMatch* DeathMatchGameState = GetGameState<AHTGSDeathMatch>();
	if (DeathMatchGameState)
	{
		DeathMatchGameState->SetTimeRemaining(GameRestartDelay);
	}
	GetWorldTimerManager().SetTimer(GameTimerHandle, this, &AHTGMDeathMatch::OnOneSecondElapsed, 1.f, true);
	
	FTimerHandle GameRestartTimerHandle;
	GetWorldTimerManager().SetTimer(GameRestartTimerHandle, this, &AHTGMDeathMatch::ResetLevel, GameRestartDelay, true);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (PlayerController)
		{
			AHTHoverTank* PossessedHoverTank = Cast<AHTHoverTank>(PlayerController->GetPawn());

			if (PossessedHoverTank)
			{
				PossessedHoverTank->SetInputEnabled(false);
			}

			IHTHasScoreBoardInterface* ControllerWithScoreBoard = Cast<IHTHasScoreBoardInterface>(PlayerController);
			if (ControllerWithScoreBoard)
			{
				ControllerWithScoreBoard->ClientForceOpenScoreBoard(GameRestartDelay);
			}
		}
	}
}

void AHTGMDeathMatch::ResetLevel()
{
	GetWorldTimerManager().ClearTimer(GameTimerHandle);

	// UE_LOG(LogTemp, Warning, TEXT("ResetLevel with ?Restart"));
	// Super::ResetLevel();

	GetWorld()->ServerTravel("?Restart",false);
}

void AHTGMDeathMatch::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AHTGSDeathMatch* DeathMatchGameState = GetGameState<AHTGSDeathMatch>();
	if (DeathMatchGameState)
	{
		DeathMatchGameState->InitializeNewPlayerScore(NewPlayer);
	}
}

void AHTGMDeathMatch::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// UE_LOG(LogTemp, Warning, TEXT("DeathMatchGameMode Logout happened for %s"), *Exiting->GetName());

	// is World tearing down?
	if (GetWorld()->bIsTearingDown || !GEngine || !GEngine->IsInitialized())
	{
		return;
	}

	/**
	 * Major workaround, handle scoreboard refresh on logout after a timer.
	 *
	 * Logout happens even on editor shutdown. The GameState can still handle the removal, but by the time it replicates
	 * the PlayerScores and the PlayerControllers want to refresh, the World and the GameState references are already
	 * destroyed and they crash.
	 *
	 * The timer get's destroyed as well on editor shutdown, so the totatlly irrelevant scoreboard refresh won't happen.
	 */
	
	const FString ExitingPlayerName = Exiting->PlayerState->GetPlayerName();
	const FTimerDelegate ScoreBoardRefreshDelegate = FTimerDelegate::CreateUObject(this, &AHTGMDeathMatch::RemovePlayerFromScoreBoardOnLogout, ExitingPlayerName);
	GetWorldTimerManager().SetTimer(OnLogoutScoreRefreshTimerHandle, ScoreBoardRefreshDelegate, 1.f, false);
}

APlayerStart* AHTGMDeathMatch::FindRandomSpawnPoint()
{
	const int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
	APlayerStart* RandomSpawnPoint = SpawnPoints[RandomIndex];

	return RandomSpawnPoint;
}

AHTHoverTank* AHTGMDeathMatch::SpawnTankAtPlayerStart(APlayerStart* RandomSpawnPoint)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHTHoverTank* NewHoverTank = GetWorld()->SpawnActor<AHTHoverTank>(
		DefaultPawnClass,
		RandomSpawnPoint->GetActorLocation(),
		RandomSpawnPoint->GetActorRotation(),
		SpawnParams
	);

	return NewHoverTank;
}

void AHTGMDeathMatch::RemovePlayerFromScoreBoardOnLogout(const FString PlayerName)
{
	AHTGSDeathMatch* DeathMatchGameState = GetGameState<AHTGSDeathMatch>();
	if (DeathMatchGameState)
	{
		DeathMatchGameState->RemovePlayersScore(PlayerName);
	}
}

void AHTGMDeathMatch::SomeoneKilledSomeone(AController* KillerController, AController* VictimController)
{
	const FString KillerName = KillerController->PlayerState->GetPlayerName();
	const FString VictimName = VictimController->PlayerState->GetPlayerName();

	// iterate over every joined player, cast their PlayerControllers to AHTPlayerController and call ClientAddKillIndicator
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHTPlayerController* PlayerController = Cast<AHTPlayerController>(It->Get());
		if (PlayerController)
		{
			PlayerController->ClientAddKillIndicator(KillerName, VictimName);
		}
	}
}
