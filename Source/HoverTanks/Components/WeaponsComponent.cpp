// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponsComponent.h"

#include "HoverTanks/HoverTank.h"
#include "HoverTanks/TankProjectile.h"

// Sets default values for this component's properties
UWeaponsComponent::UWeaponsComponent(): TankBarrelMesh(nullptr)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	ProjectileClass = ATankProjectile::StaticClass();
}


// Called when the game starts
void UWeaponsComponent::BeginPlay()
{
	Super::BeginPlay();

	AHoverTank* HoverTank = Cast<AHoverTank>(GetOwner());
	if (HoverTank)
	{
		TankBarrelMesh = HoverTank->GetTankBarrelMesh();
	}
	
}

// Called every frame
void UWeaponsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeaponsComponent::AttemptToShoot()
{
	APawn* Owner = Cast<APawn>(GetOwner());
	if ((Owner && Owner->IsLocallyControlled()) || GetOwnerRole() == ROLE_AutonomousProxy)
	{
		ServerAttemptToShoot();
	}
}


void UWeaponsComponent::ServerAttemptToShoot_Implementation()
{
	SpawnProjectile();
}

bool UWeaponsComponent::ServerAttemptToShoot_Validate()
{
	return true;
}

void UWeaponsComponent::SpawnProjectile()
{
	if (TankBarrelMesh)
	{
		FVector BarrelEndLocation = TankBarrelMesh->GetSocketLocation(FName("BarrelEnd"));
		FRotator BarrelEndRotation = TankBarrelMesh->GetSocketRotation(FName("BarrelEnd"));

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();
		SpawnParameters.Instigator = GetOwner()->GetInstigator();

		ATankProjectile* Projectile = GetWorld()->SpawnActor<ATankProjectile>(ProjectileClass, BarrelEndLocation, BarrelEndRotation, SpawnParameters);
	}	
}