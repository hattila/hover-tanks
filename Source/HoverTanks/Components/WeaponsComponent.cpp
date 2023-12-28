// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponsComponent.h"

#include "HoverTanks/HoverTank.h"
#include "HoverTanks/Actors/Projectiles/CannonProjectile.h"
#include "HoverTanks/Actors/Weapons/RocketLauncher.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UWeaponsComponent::UWeaponsComponent(): TankCannonMesh(nullptr), TankBarrelMesh(nullptr)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	ProjectileClass = ACannonProjectile::StaticClass();
}

void UWeaponsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UWeaponsComponent::SwitchToNextWeapon()
{
	if (CurrentWeapon == EAvailableWeapons::Cannon)
	{
		CurrentWeapon = EAvailableWeapons::RocketLauncher;
	}
	else
	{
		CurrentWeapon = EAvailableWeapons::Cannon;
	}

	AHoverTank* HoverTank = Cast<AHoverTank>(GetOwner());
	if (HoverTank == nullptr)
	{
		return;
	}

	HoverTank->OnWeaponSwitched.Broadcast(CurrentWeapon);
}

void UWeaponsComponent::SwitchToPrevWeapon()
{
	if (CurrentWeapon == EAvailableWeapons::Cannon)
	{
		CurrentWeapon = EAvailableWeapons::RocketLauncher;
	}
	else
	{
		CurrentWeapon = EAvailableWeapons::Cannon;
	}

	AHoverTank* HoverTank = Cast<AHoverTank>(GetOwner());
	if (HoverTank == nullptr)
	{
		return;
	}

	HoverTank->OnWeaponSwitched.Broadcast(CurrentWeapon);
}

void UWeaponsComponent::MulticastDestroyAttachedWeapons_Implementation()
{
	if (RocketLauncher)
	{
		RocketLauncher->Destroy();
	}
}

// Called when the game starts
void UWeaponsComponent::BeginPlay()
{
	Super::BeginPlay();

	AHoverTank* HoverTank = Cast<AHoverTank>(GetOwner());
	if (HoverTank)
	{
		TankCannonMesh = HoverTank->GetTankCannonMesh();
		TankBarrelMesh = HoverTank->GetTankBarrelMesh();

		CreateAndAttachRocketLauncher();
	}
	
}

// Called every frame
void UWeaponsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	if (ClientRocketLauncherTarget.IsValidBlockingHit())
	{
		ShowRocketTarget(ClientRocketLauncherTarget);
	}

	// ...
}

// void UWeaponsComponent::AttemptToShoot(const FVector& LocationUnderTheCrosshair)
void UWeaponsComponent::AttemptToShoot(const FHitResult& Hit)
{
	APawn* Owner = Cast<APawn>(GetOwner());
	if ((Owner && Owner->IsLocallyControlled()) || GetOwnerRole() == ROLE_AutonomousProxy)
	{
		switch (CurrentWeapon)
		{
			case EAvailableWeapons::Cannon:
				ServerAttemptToShoot();
				break;
			case EAvailableWeapons::RocketLauncher:
				// ServerAttemptToShootRocketLauncher(LocationUnderTheCrosshair);
				ServerAttemptToShootRocketLauncher(Hit);
				break;
			default:
			return;
		}
		
		// ServerAttemptToShoot();
		// ServerAttemptToShootRocketLauncher(LocationUnderTheCrosshair);
	}
}


void UWeaponsComponent::ServerAttemptToShoot_Implementation()
{
	if (bIsMainCannonOnCooldown)
	{
		return;
	}

	SpawnProjectile();
	bIsMainCannonOnCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(MainCannonCooldownTimerHandle, this, &UWeaponsComponent::ClearMainCannonCooldown, 1.f, false, 1.f);
}

bool UWeaponsComponent::ServerAttemptToShoot_Validate()
{
	return true;
}

void UWeaponsComponent::ClearMainCannonCooldown()
{
	bIsMainCannonOnCooldown = false;
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

		ACannonProjectile* Projectile = GetWorld()->SpawnActor<ACannonProjectile>(ProjectileClass, BarrelEndLocation, BarrelEndRotation, SpawnParameters);
	}	
}

void UWeaponsComponent::CreateAndAttachRocketLauncher()
{
	// get the socket location and rotation of LeftMount socket on TankCannonMesh
	FVector LeftMountLocation = TankCannonMesh->GetSocketLocation(FName("LeftMount"));
	FRotator LeftMountRotation = TankCannonMesh->GetSocketRotation(FName("LeftMount"));

	LeftMountRotation = LeftMountRotation + FRotator(60.f, 0.f, 0.f);
	// UE_LOG(LogTemp, Warning, TEXT("LeftMountRotation: %s"), *LeftMountRotation.ToString());

	// spawn the RocketLauncher
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = GetOwner()->GetInstigator();

	// create a RocketLauncher Actor, and attach it to teh TankCannonMesh, LeftMount socket
	RocketLauncher = GetWorld()->SpawnActor<ARocketLauncher>(ARocketLauncher::StaticClass(), LeftMountLocation, LeftMountRotation, SpawnParameters);
	RocketLauncher->AttachToComponent(TankCannonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("LeftMount"));
	RocketLauncher->SetActorRotation(LeftMountRotation);
	
}

void UWeaponsComponent::ServerAttemptToShootRocketLauncher_Implementation(const FHitResult& Hit)
{
	if (RocketLauncher == nullptr)
	{
		return;
	}

	RocketLauncher->SetRocketTargetHitResult(Hit);
	MulticastShowRocketTarget(Hit);

	RocketLauncher->Fire();
}

bool UWeaponsComponent::ServerAttemptToShootRocketLauncher_Validate(const FHitResult& Hit)
{
	return true;
}

void UWeaponsComponent::MulticastShowRocketTarget_Implementation(const FHitResult& Hit)
{
	ClientRocketLauncherTarget = Hit;
}

void UWeaponsComponent::ShowRocketTarget(const FHitResult& Hit) const
{
	if (Hit.IsValidBlockingHit())
	{
		FVector HitLocation;
		
		if (AHoverTank* HoverTank = Cast<AHoverTank>(Hit.GetActor()))
		{
			HitLocation = HoverTank->GetActorLocation();
		}
		else
		{
			HitLocation = Hit.Location;
		}

		// show the debug sphere to the owning client only
		// ShowTargetIndicator(HitLocation);
		DrawDebugSphere(GetWorld(), HitLocation,200,12, FColor::Red,false,0);
	}
}
