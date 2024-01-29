// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameMode.h"

#include "HoverTanks/Game/GameStates/DeathMatchGameState.h"
#include "HoverTanks/Pawns/HoverTank.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "HoverTanks/Controllers/HoverTankPlayerController.h"
#include "HoverTanks/Game/HTPlayerState.h"
#include "HoverTanks/UI/HUD/DeathMatchHUD.h"

#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ADeathMatchGameMode::ADeathMatchGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/HoverTanks/Pawns/HoverTank/BP_HoverTank"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PlayerControllerClass = AHoverTankPlayerController::StaticClass();

	TArray<AActor*> SpawnPointsInWorld;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPointsInWorld);
	for (AActor* SpawnPoint : SpawnPointsInWorld)
	{
		SpawnPoints.Add(Cast<APlayerStart>(SpawnPoint));
	}

	// Custom GameInstance should be set in DefaultEngine.ini

	PlayerStateClass = AHTPlayerState::StaticClass();
	GameStateClass = ADeathMatchGameState::StaticClass();
	HUDClass = ADeathMatchHUD::StaticClass();

	bUseSeamlessTravel = false;
}

void ADeathMatchGameMode::TankDies(AHoverTank* DeadHoverTank, AController* DeathCauser)
{
	// UE_LOG(LogTemp, Warning, TEXT("Tank %s died!"), *DeadHoverTank->GetName());

	APlayerController* DeadPlayerController = Cast<APlayerController>(DeadHoverTank->GetController());
	if (DeadPlayerController)
	{
		SomeoneKilledSomeone(DeathCauser, DeadPlayerController);

		APlayerController* KillerPlayerController = Cast<APlayerController>(DeathCauser);
		
		ADeathMatchGameState* DeathMatchGameState = GetGameState<ADeathMatchGameState>();
		if (DeathMatchGameState && KillerPlayerController != nullptr)
		{
			DeathMatchGameState->AddScoreToPlayer(KillerPlayerController, 1);
		}

		DeadHoverTank->OnDeath();
	}
	
}

void ADeathMatchGameMode::RequestRespawn(APlayerController* InPlayerController)
{
	ADeathMatchGameState* DeathMatchGameState = GetGameState<ADeathMatchGameState>();

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
				UE_LOG(LogTemp, Warning, TEXT("ADeathMatchGameMode::RequestRespawn - CurrentPawn is null"));
			}

			APlayerStart* RandomSpawnPoint = FindRandomSpawnPoint();
			AHoverTank* NewHoverTank = SpawnTankAtPlayerStart(RandomSpawnPoint);

			InPlayerController->Possess(NewHoverTank);
		}
	}
}

void ADeathMatchGameMode::BeginPlay()
{
	Super::BeginPlay();
	// UE_LOG(LogTemp, Warning, TEXT("DeathMatchGameMode BeginPlay %d"), MatchTimeInSeconds);

	ADeathMatchGameState* DeathMatchGameState = GetGameState<ADeathMatchGameState>();
	if (DeathMatchGameState)
	{
		DeathMatchGameState->SetTimeRemaining(MatchTimeInSeconds);
	}

	GetWorldTimerManager().SetTimer(GameTimerHandle, this, &ADeathMatchGameMode::OnOneSecondElapsed, 1.f, true);

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

void ADeathMatchGameMode::OnOneSecondElapsed()
{
	ADeathMatchGameState* DeathMatchGameState = GetGameState<ADeathMatchGameState>();
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
void ADeathMatchGameMode::GameOver()
{
	// clear the timer
	GetWorldTimerManager().ClearTimer(GameTimerHandle);
	MatchState = EMatchState::GameOver;
	const float GameRestartDelay = 10.f;

	// UE_LOG(LogTemp, Warning, TEXT("GameOver!"));

	ADeathMatchGameState* DeathMatchGameState = GetGameState<ADeathMatchGameState>();
	if (DeathMatchGameState)
	{
		DeathMatchGameState->SetTimeRemaining(GameRestartDelay);
	}
	GetWorldTimerManager().SetTimer(GameTimerHandle, this, &ADeathMatchGameMode::OnOneSecondElapsed, 1.f, true);
	
	FTimerHandle GameRestartTimerHandle;
	GetWorldTimerManager().SetTimer(GameRestartTimerHandle, this, &ADeathMatchGameMode::ResetLevel, GameRestartDelay, true);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (PlayerController)
		{
			AHoverTank* PossessedHoverTank = Cast<AHoverTank>(PlayerController->GetPawn());

			if (PossessedHoverTank)
			{
				PossessedHoverTank->SetInputEnabled(false);
			}

			IHasScoreBoardController* HoverTankPlayerController = Cast<IHasScoreBoardController>(PlayerController);
			if (HoverTankPlayerController)
			{
				HoverTankPlayerController->ClientForceOpenScoreBoard(GameRestartDelay);
			}
		}
	}
}

void ADeathMatchGameMode::ResetLevel()
{
	GetWorldTimerManager().ClearTimer(GameTimerHandle);

	// UE_LOG(LogTemp, Warning, TEXT("ResetLevel with ?Restart"));
	// Super::ResetLevel();

	GetWorld()->ServerTravel("?Restart",false);
}

void ADeathMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ADeathMatchGameState* DeathMatchGameState = GetGameState<ADeathMatchGameState>();
	if (DeathMatchGameState)
	{
		DeathMatchGameState->InitializeNewPlayerScore(NewPlayer);
	}
}

void ADeathMatchGameMode::Logout(AController* Exiting)
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
	const FTimerDelegate ScoreBoardRefreshDelegate = FTimerDelegate::CreateUObject(this, &ADeathMatchGameMode::RemovePlayerFromScoreBoardOnLogout, ExitingPlayerName);
	GetWorldTimerManager().SetTimer(OnLogoutScoreRefreshTimerHandle, ScoreBoardRefreshDelegate, 1.f, false);
}

APlayerStart* ADeathMatchGameMode::FindRandomSpawnPoint()
{
	const int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
	APlayerStart* RandomSpawnPoint = SpawnPoints[RandomIndex];

	return RandomSpawnPoint;
}

AHoverTank* ADeathMatchGameMode::SpawnTankAtPlayerStart(APlayerStart* RandomSpawnPoint)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHoverTank* NewHoverTank = GetWorld()->SpawnActor<AHoverTank>(
		DefaultPawnClass,
		RandomSpawnPoint->GetActorLocation(),
		RandomSpawnPoint->GetActorRotation(),
		SpawnParams
	);

	return NewHoverTank;
}

void ADeathMatchGameMode::RemovePlayerFromScoreBoardOnLogout(const FString PlayerName)
{
	ADeathMatchGameState* DeathMatchGameState = GetGameState<ADeathMatchGameState>();
	if (DeathMatchGameState)
	{
		DeathMatchGameState->RemovePlayersScore(PlayerName);
	}
}

void ADeathMatchGameMode::SomeoneKilledSomeone(AController* KillerController, AController* VictimController)
{
	const FString KillerName = KillerController->PlayerState->GetPlayerName();
	const FString VictimName = VictimController->PlayerState->GetPlayerName();

	// iterate over every joined player, cast their PlayerControllers to AHoverTankPlayerController and call ClientAddKillIndicator
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHoverTankPlayerController* PlayerController = Cast<AHoverTankPlayerController>(It->Get());
		if (PlayerController)
		{
			PlayerController->ClientAddKillIndicator(KillerName, VictimName);
		}
	}
}
