// Fill out your copyright notice in the Description page of Project Settings.


#include "HTWeaponsComponent.h"

#include "HoverTanks/Pawns/HoverTank.h"
#include "HoverTanks/Actors/Projectiles/HTCannonProjectile.h"
#include "HoverTanks/Actors/Weapons/HTRocketLauncher.h"
#include "Kismet/GameplayStatics.h"

UHTWeaponsComponent::UHTWeaponsComponent(): TankCannonMesh(nullptr), TankBarrelMesh(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHTWeaponsComponent::BeginPlay()
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

void UHTWeaponsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UHTWeaponsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ClientRocketLauncherTarget.IsValidBlockingHit())
	{
		ShowRocketTarget(ClientRocketLauncherTarget);
	}

}

void UHTWeaponsComponent::SwitchToNextWeapon()
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

void UHTWeaponsComponent::SwitchToPrevWeapon()
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

void UHTWeaponsComponent::MulticastDestroyAttachedWeapons_Implementation()
{
	if (RocketLauncher)
	{
		RocketLauncher->Destroy();
	}
}

void UHTWeaponsComponent::AttemptToShoot(const FHitResult& Hit)
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


void UHTWeaponsComponent::ServerAttemptToShoot_Implementation()
{
	if (bIsMainCannonOnCooldown)
	{
		return;
	}

	SpawnProjectile();
	bIsMainCannonOnCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(MainCannonCooldownTimerHandle, this, &UHTWeaponsComponent::ClearMainCannonCooldown, 1.f, false, 1.f);

	ClientOnFire(0, 1);
}

bool UHTWeaponsComponent::ServerAttemptToShoot_Validate()
{
	return true;
}

void UHTWeaponsComponent::ClearMainCannonCooldown()
{
	bIsMainCannonOnCooldown = false;
}

void UHTWeaponsComponent::SpawnProjectile()
{
	if (CannonProjectileClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHTWeaponsComponent::SpawnProjectile - ProjectileClass is null"));
		return;
	}

	if (TankBarrelMesh == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHTWeaponsComponent::SpawnProjectile - TankBarrelMesh is null"));
		return;
	}

	FVector BarrelEndLocation = TankBarrelMesh->GetSocketLocation(FName("BarrelEnd"));
	FRotator BarrelEndRotation = TankBarrelMesh->GetSocketRotation(FName("BarrelEnd"));

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = GetOwner()->GetInstigator();

	AHTCannonProjectile* Projectile = GetWorld()->SpawnActor<AHTCannonProjectile>(CannonProjectileClass, BarrelEndLocation, BarrelEndRotation, SpawnParameters);
	
}

void UHTWeaponsComponent::CreateAndAttachRocketLauncher()
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
	// RocketLauncher = GetWorld()->SpawnActor<ARocketLauncher>(ARocketLauncher::StaticClass(), LeftMountLocation, LeftMountRotation, SpawnParameters);

	// @see https://forums.unrealengine.com/t/spawning-an-actor-with-parameters/329151/4
	RocketLauncher = Cast<AHTRocketLauncher>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, AHTRocketLauncher::StaticClass(), FTransform(LeftMountRotation, LeftMountLocation)));
	if (RocketLauncher != nullptr)
	{
		RocketLauncher->Init(RocketProjectileClass);
		UGameplayStatics::FinishSpawningActor(RocketLauncher, FTransform(LeftMountRotation, LeftMountLocation));
		// set the owner and instigator
		RocketLauncher->SetOwner(GetOwner());
		RocketLauncher->SetInstigator(GetOwner()->GetInstigator());
	}
	
	
	RocketLauncher->AttachToComponent(TankCannonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("LeftMount"));
	RocketLauncher->SetActorRotation(LeftMountRotation);
	
}

void UHTWeaponsComponent::ServerAttemptToShootRocketLauncher_Implementation(const FHitResult& Hit)
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

bool UHTWeaponsComponent::ServerAttemptToShootRocketLauncher_Validate(const FHitResult& Hit)
{
	return true;
}

void UHTWeaponsComponent::MulticastShowRocketTarget_Implementation(const FHitResult& Hit)
{
	ClientRocketLauncherTarget = Hit;

	FTimerHandle ClearTargetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(ClearTargetTimerHandle, this, &UHTWeaponsComponent::ClearRocketTarget, 2.9f, false, 2.9f);
}

void UHTWeaponsComponent::ShowRocketTarget(const FHitResult& Hit) const
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

void UHTWeaponsComponent::ClearRocketTarget()
{
	ClientRocketLauncherTarget = FHitResult();
}


void UHTWeaponsComponent::ClientOnFire_Implementation(int32 WeaponIndex, float CooldownTime)
{
	OnWeaponFire.Broadcast(WeaponIndex, CooldownTime);
}