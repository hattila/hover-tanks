// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketLauncher.h"

#include "HoverTanks/HoverTank.h"
#include "HoverTanks/Actors/Projectiles/RocketProjectile.h"


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

	UE_LOG(LogTemp, Warning, TEXT("ARocketLauncher::Fire(), RocketTargetLocation is %s"), *RocketTargetLocation.ToString());

	// if RocketTargetLocation is not a null vector
	if (RocketTargetLocation == FVector::ZeroVector)
	{
		RocketTargetLocationComponent = nullptr;
	}
	else
	{
		RocketTargetLocationComponent = NewObject<USceneComponent>(this, TEXT("RocketTargetLocationComponent"));
		RocketTargetLocationComponent->SetWorldLocation(RocketTargetLocation);

		// draw a debug sphere at RocketTargetLocation
		DrawDebugSphere(
			GetWorld(),
			RocketTargetLocation,
			200,
			12,
			FColor::Blue,
			false,
			2
		);
	}
	

	// create TimerHandle params
	FTimerDelegate FireTimerDelegate = FTimerDelegate::CreateUObject(
		this,
		&ARocketLauncher::BurstFire,
		RocketTargetLocationComponent
	);
	
	// UE_LOG(LogTemp, Warning, TEXT("ARocketLauncher::Fire(), my Owner is %s"), *GetOwner()->GetName());

	// GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &ARocketLauncher::BurstFire, TimeBetweenShots, true, 0);
	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, FireTimerDelegate, TimeBetweenShots, true, 0);
	
	// SpawnProjectile();
	bIsOnCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(FireCooldownTimerHandle, this, &ARocketLauncher::ClearFireCooldownTimer, FireCooldownTime, false, FireCooldownTime);
}

void ARocketLauncher::ClearFireCooldownTimer()
{
	bIsOnCooldown = false;
	// UE_LOG(LogTemp, Warning, TEXT("ARocketLauncher::ClearFireCooldownTimer()"));
}

void ARocketLauncher::BurstFire(USceneComponent* InRocketTargetLocationComponent)
{
	if (CurrentFireCount >= MaxBurstFireCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
		CurrentFireCount = 0;
		return;
	}

	SpawnProjectile(InRocketTargetLocationComponent);
	CurrentFireCount++;
}

/**
 * Issue: Rotation follow velocity does not work on Clients. The internal Velocity of the ProjectileMovementComponent is
 * not replicated to Clients. The only way I found around this is to spawn the projectile on the clients as well, and do
 * not replicate the movement component. This is not viable however because of the changed properties on the server.
 */
void ARocketLauncher::SpawnProjectile(USceneComponent* InRocketTargetLocationComponent)
{
	if (!HasAuthority())
	{
		return;
	}

	// create a transform form SpawnLocation and SpawnRotation
	FTransform SpawnTransform = FTransform(GetActorRotation(), GetActorLocation());

	ARocketProjectile* Projectile = GetWorld()->SpawnActorDeferred<ARocketProjectile>(ARocketProjectile::StaticClass(), SpawnTransform, GetOwner(), GetOwner()->GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (InRocketTargetLocationComponent)
	{
		Projectile->SetRocketTargetLocationComponent(InRocketTargetLocationComponent);
	}
	
	Projectile->FinishSpawning(SpawnTransform);
}