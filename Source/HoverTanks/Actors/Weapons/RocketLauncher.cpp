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
	RootComponent = BaseMesh;

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

	if (RocketTargetHitResult.IsValidBlockingHit())
	{
		FVector HitLocation;
		
		if (AHoverTank* HoverTank = Cast<AHoverTank>(RocketTargetHitResult.GetActor()))
		{
			HitLocation = HoverTank->GetActorLocation();
		}
		else
		{
			HitLocation = RocketTargetHitResult.Location;
		}

		DrawDebugSphere(GetWorld(), HitLocation,200,12, FColor::Blue,false,0);
	}
}

void ARocketLauncher::Fire()
{
	if (bIsOnCooldown)
	{
		return;
	}

	
	FTimerDelegate FireTimerDelegate = FTimerDelegate::CreateUObject(
		this,
		&ARocketLauncher::BurstFire
	);

	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, FireTimerDelegate, TimeBetweenShots, true, 0);

	bIsOnCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(FireCooldownTimerHandle, this, &ARocketLauncher::ClearFireCooldownTimer, FireCooldownTime, false, FireCooldownTime);
}

void ARocketLauncher::ClearFireCooldownTimer()
{
	bIsOnCooldown = false;
	// UE_LOG(LogTemp, Warning, TEXT("ARocketLauncher::ClearFireCooldownTimer()"));
}

void ARocketLauncher::BurstFire()
{
	if (CurrentFireCount >= MaxBurstFireCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
		CurrentFireCount = 0;
		return;
	}

	SpawnProjectile(RocketTargetHitResult);
	CurrentFireCount++;
}

/**
 * Issue: Rotation follow velocity does not work on Clients. The internal Velocity of the ProjectileMovementComponent is
 * not replicated to Clients. The only way I found around this is to spawn the projectile on the clients as well, and do
 * not replicate the movement component. This is not viable however because of the changed properties on the server.
 */
void ARocketLauncher::SpawnProjectile(const FHitResult& InTargetHitResult) const
{
	if (!HasAuthority())
	{
		return;
	}

	FTransform SpawnTransform = FTransform(GetActorRotation(), GetActorLocation());
	ARocketProjectile* Projectile = GetWorld()->SpawnActorDeferred<ARocketProjectile>(ARocketProjectile::StaticClass(), SpawnTransform, GetOwner(), GetOwner()->GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	
	Projectile->SetRocketTargetHitResult(InTargetHitResult);
	Projectile->FinishSpawning(SpawnTransform);
}
