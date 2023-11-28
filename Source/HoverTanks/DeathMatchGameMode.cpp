// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameMode.h"

#include "HoverTank.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ADeathMatchGameMode::ADeathMatchGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/HoverTanks/HoverTank/BP_HoverTank"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	
	// find all players starts in the map and add them to the spawn points array
	TArray<AActor*> SpawnPointsInWorld;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPointsInWorld);

	for (AActor* SpawnPoint : SpawnPointsInWorld)
	{
		SpawnPoints.Add(Cast<APlayerStart>(SpawnPoint));
	}
	
}

void ADeathMatchGameMode::TankDies(AHoverTank* DeadHoverTank)
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

		DeadPlayerController->UnPossess();
		DeadHoverTank->Destroy();
		
		APlayerStart* RandomSpawnPoint = FindRandomSpawnPoint();
		AHoverTank* NewHoverTank = SpawnTankAtPlayerStart(RandomSpawnPoint);

		DeadPlayerController->Possess(NewHoverTank);
	}
	
}

void ADeathMatchGameMode::BeginPlay()
{
	Super::BeginPlay();
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
