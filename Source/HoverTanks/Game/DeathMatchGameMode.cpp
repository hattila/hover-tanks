// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameMode.h"

#include "DeathMatchGameState.h"
#include "HoverTanks/HoverTank.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "HoverTanks/HoverTankPlayerController.h"
#include "HoverTanks/UI/HUD/DeathMatchHUD.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ADeathMatchGameMode::ADeathMatchGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/HoverTanks/HoverTank/BP_HoverTank"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// find the BP_HoverTankController blueprint class
	// static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/HoverTanks/Controllers/BP_HoverTankController"));
	// if (PlayerControllerBPClass.Class != NULL)
	// {
	// 	PlayerControllerClass = PlayerControllerBPClass.Class;
	// }

	// set the default PlayerController to be HoverTankPlayerController
	PlayerControllerClass = AHoverTankPlayerController::StaticClass();
	
	// find all players starts in the map and add them to the spawn points array
	TArray<AActor*> SpawnPointsInWorld;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPointsInWorld);

	for (AActor* SpawnPoint : SpawnPointsInWorld)
	{
		SpawnPoints.Add(Cast<APlayerStart>(SpawnPoint));
	}

	// Custom GameInstance should be set in DefaultEngine.ini

	// use the UDeathMatchGameState as default GameState
	GameStateClass = ADeathMatchGameState::StaticClass();

	// use the DeathMatchHUD as default HUD
	HUDClass = ADeathMatchHUD::StaticClass();

	bUseSeamlessTravel = true;
}

void ADeathMatchGameMode::TankDies(AHoverTank* DeadHoverTank, AController* DeathCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("Tank %s died!"), *DeadHoverTank->GetName());

	APlayerController* DeadPlayerController = Cast<APlayerController>(DeadHoverTank->GetController());
	if (DeadPlayerController)
	{
		// Kill indicator
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.f,
			FColor::Green,
			FString::Printf(
				TEXT("%s died!"),
				*DeadPlayerController->PlayerState->GetPlayerName()
			)
		);

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
		if (TimeRemaining > 0)
		{
			APawn* CurrentPawn = InPlayerController->GetPawn();
			InPlayerController->UnPossess();
			if (CurrentPawn)
			{
				CurrentPawn->Destroy();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("CurrentPawn is null"));
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
	UE_LOG(LogTemp, Warning, TEXT("DeathMatchGameMode BeginPlay %d"), MatchTimeInSeconds);

	ADeathMatchGameState* DeathMatchGameState = GetGameState<ADeathMatchGameState>();
	if (DeathMatchGameState)
	{
		DeathMatchGameState->SetTimeRemaining(MatchTimeInSeconds);
	}

	GetWorldTimerManager().SetTimer(GameTimerHandle, this, &ADeathMatchGameMode::OnOneSecondElapsed, 1.f, true);
}

void ADeathMatchGameMode::OnOneSecondElapsed()
{
	ADeathMatchGameState* DeathMatchGameState = GetGameState<ADeathMatchGameState>();
	if (DeathMatchGameState)
	{
		const int32 TimeRemaining = DeathMatchGameState->GetTimeRemaining();
		DeathMatchGameState->SetTimeRemaining(TimeRemaining - 1);

		if (TimeRemaining <= 0)
		{
			GameOver();
		}
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("One second elapsed!"));

	// todo check if the game is over
}

void ADeathMatchGameMode::GameOver()
{
	// clear the timer
	GetWorldTimerManager().ClearTimer(GameTimerHandle);

	// get every connected palyer controller, find their possesed pawns and destroy them
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (PlayerController)
		{
			AHoverTank* PossessedHoverTank = Cast<AHoverTank>(PlayerController->GetPawn());

			// kill all tanks and open the scoreboard
			if (PossessedHoverTank)
			{
				// UGameplayStatics::ApplyDamage(
				// 	PossessedHoverTank,
				// 	5000.f,
				// 	nullptr,
				// 	this,
				// 	UDamageType::StaticClass()
				// );
				PossessedHoverTank->SetInputEnabled(false);
			}

			AHoverTankPlayerController* HoverTankPlayerController = Cast<AHoverTankPlayerController>(PlayerController);
			if (HoverTankPlayerController)
			{
				HoverTankPlayerController->ClientForceOpenScoreBoard(); // clients!
			}
		}
	}

	GetWorldTimerManager().SetTimer(GameTimerHandle, this, &ADeathMatchGameMode::ResetLevel, 2.f, true);
}

void ADeathMatchGameMode::ResetLevel()
{
	GetWorldTimerManager().ClearTimer(GameTimerHandle);

	UE_LOG(LogTemp, Warning, TEXT("ResetLevel with ?Restart"));
	// Super::ResetLevel();

	GetWorld()->ServerTravel("?Restart",false);
}

void ADeathMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Add a new entry to PlayerScores array in DeathMatchGameState for the newly logged in player
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

	ADeathMatchGameState* DeathMatchGameState = GetGameState<ADeathMatchGameState>();
	const APlayerController* ExistingPlayerController = Cast<APlayerController>(Exiting);
	if (DeathMatchGameState && ExistingPlayerController)
	{
		DeathMatchGameState->RemovePlayersScore(ExistingPlayerController);
	}
}

APlayerStart* ADeathMatchGameMode::FindRandomSpawnPoint()
{
	int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
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
