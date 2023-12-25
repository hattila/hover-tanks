// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketLauncher.h"

#include "HoverTanks/Components/WeaponsComponent.h"
#include "..\Projectiles\CannonProjectile.h"


// Sets default values
ARocketLauncher::ARocketLauncher()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketLauncherMesh"));
	BaseMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BaseMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	UStaticMesh* BaseMeshObject = BaseMeshAsset.Object;
	BaseMesh->SetStaticMesh(BaseMeshObject);
	BaseMesh->SetCollisionProfileName(TEXT("NoCollision"));
}

// Called when the game starts or when spawned
void ARocketLauncher::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARocketLauncher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARocketLauncher::Fire()
{
	if (bIsOnCooldown)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ARocketLauncher::Fire()"));

	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &ARocketLauncher::BurstFire, TimeBetweenShots, true, 0);
	// SpawnProjectile();
	bIsOnCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(FireCooldownTimerHandle, this, &ARocketLauncher::ClearFireCooldownTimer, FireCooldownTime, false, FireCooldownTime);
}

void ARocketLauncher::ClearFireCooldownTimer()
{
	bIsOnCooldown = false;
	UE_LOG(LogTemp, Warning, TEXT("ARocketLauncher::ClearFireCooldownTimer()"));
}

void ARocketLauncher::BurstFire()
{
	if (CurrentFireCount >= MaxBurstFireCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
		CurrentFireCount = 0;
		return;
	}

	SpawnProjectile();
	CurrentFireCount++;
}

void ARocketLauncher::SpawnProjectile()
{
	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = GetOwner()->GetInstigator();

	ACannonProjectile* Projectile = GetWorld()->SpawnActor<ACannonProjectile>(ACannonProjectile::StaticClass(), SpawnLocation, SpawnRotation, SpawnParameters);
}
