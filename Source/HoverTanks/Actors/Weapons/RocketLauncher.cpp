// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketLauncher.h"

#include "HoverTanks/Actors/Projectiles/RocketProjectile.h"


ARocketLauncher::ARocketLauncher()
{
	PrimaryActorTick.bCanEverTick = false;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketLauncherMesh"));
	RootComponent = BaseMesh;

	// static ConstructorHelpers::FObjectFinder<UStaticMesh> BaseMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BaseMeshAsset(TEXT("/Game/HoverTanks/Pawns/TankRocketLauncher"));
	UStaticMesh* BaseMeshObject = BaseMeshAsset.Object;
	BaseMesh->SetStaticMesh(BaseMeshObject);
	BaseMesh->SetCollisionProfileName(TEXT("NoCollision"));
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/**
	 * Materials
	 */
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TankBaseMaterialAsset(TEXT("/Game/Megascans/surfaces/Painted_Gun_Metal_shrbehqc/MI_Painted_Gun_Metal_shrbehqc_4K"));
	UMaterialInterface* TankBaseMaterialAssetObject = TankBaseMaterialAsset.Object;
	BaseMesh->SetMaterial(0, TankBaseMaterialAssetObject);
}

void ARocketLauncher::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARocketLauncher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ARocketLauncher::Fire()
{
	if (bIsOnCooldown)
	{
		return false;
	}
	
	FTimerDelegate FireTimerDelegate = FTimerDelegate::CreateUObject(
		this,
		&ARocketLauncher::BurstFire
	);

	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, FireTimerDelegate, TimeBetweenShots, true, 0);

	bIsOnCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(FireCooldownTimerHandle, this, &ARocketLauncher::ClearFireCooldownTimer, FireCooldownTime, false, FireCooldownTime);

	return true;
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

	// find SocketLocation named RocketSpawnPoint
	FVector RocketSpawnPoint = BaseMesh->GetSocketLocation("RocketSpawnPoint");
	
	FTransform SpawnTransform = FTransform(GetActorRotation(), RocketSpawnPoint);
	ARocketProjectile* Projectile = GetWorld()->SpawnActorDeferred<ARocketProjectile>(ARocketProjectile::StaticClass(), SpawnTransform, GetOwner(), GetOwner()->GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	
	Projectile->SetRocketTargetHitResult(InTargetHitResult);
	Projectile->FinishSpawning(SpawnTransform);
}
