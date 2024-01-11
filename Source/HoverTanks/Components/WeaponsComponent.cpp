// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponsComponent.h"

#include "HoverTanks/Pawns/HoverTank.h"
#include "HoverTanks/Actors/Projectiles/CannonProjectile.h"
#include "HoverTanks/Actors/Weapons/RocketLauncher.h"

UWeaponsComponent::UWeaponsComponent(): TankCannonMesh(nullptr), TankBarrelMesh(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;

	ProjectileClass = ACannonProjectile::StaticClass();
}

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

void UWeaponsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UWeaponsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ClientRocketLauncherTarget.IsValidBlockingHit())
	{
		ShowRocketTarget(ClientRocketLauncherTarget);
	}

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
				ServerAttemptToShootRocketLauncher(Hit);
				break;
			default:
			return;
		}
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

	ClientOnFire(0, 1);
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

	// create a RocketLauncher Actor, and attach it to the TankCannonMesh, LeftMount socket
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

	if (RocketLauncher->Fire())
	{
		ClientOnFire(1, RocketLauncher->GetFireCooldownTime());
		MulticastShowRocketTarget(Hit);
	}
}

bool UWeaponsComponent::ServerAttemptToShootRocketLauncher_Validate(const FHitResult& Hit)
{
	return true;
}

void UWeaponsComponent::MulticastShowRocketTarget_Implementation(const FHitResult& Hit)
{
	ClientRocketLauncherTarget = Hit;

	FTimerHandle ClearTargetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(ClearTargetTimerHandle, this, &UWeaponsComponent::ClearRocketTarget, 2.9f, false, 2.9f);
}

void UWeaponsComponent::ShowRocketTarget(const FHitResult& Hit) const
{
	if (Hit.IsValidBlockingHit())
	{
		FVector HitLocation;
		
		if (const AHoverTank* HoverTank = Cast<AHoverTank>(Hit.GetActor()))
		{
			HitLocation = HoverTank->GetActorLocation();
		}
		else
		{
			HitLocation = Hit.Location;
		}

		DrawDebugSphere(GetWorld(), HitLocation,150,12, FColor::Red,false,0);
	}
}

void UWeaponsComponent::ClearRocketTarget()
{
	ClientRocketLauncherTarget = FHitResult();
}


void UWeaponsComponent::ClientOnFire_Implementation(int32 WeaponIndex, float CooldownTime)
{
	OnWeaponFire.Broadcast(WeaponIndex, CooldownTime);
}