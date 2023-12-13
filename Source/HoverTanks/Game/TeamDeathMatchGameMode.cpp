// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamDeathMatchGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "HoverTanks/HoverTankPlayerController.h"
#include "Kismet/GameplayStatics.h"

ATeamDeathMatchGameMode::ATeamDeathMatchGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/HoverTanks/HoverTank/BP_HoverTank"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set the default PlayerController to be HoverTankPlayerController
	PlayerControllerClass = AHoverTankPlayerController::StaticClass();
	
	TArray<AActor*> SpawnPointsInWorld;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPointsInWorld);

	for (AActor* SpawnPoint : SpawnPointsInWorld)
	{
		SpawnPoints.Add(Cast<APlayerStart>(SpawnPoint));
	}
}

void ATeamDeathMatchGameMode::TankDies(AHoverTank* DeadHoverTank)
{
	Super::TankDies(DeadHoverTank);

	UE_LOG(LogTemp, Warning, TEXT("Tank dies in Team Death Match "));
}

void ATeamDeathMatchGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Team Death Match Begin Play"));
}
