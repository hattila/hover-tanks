// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HTWeaponsComponent.generated.h"

class AHTRocketProjectile;
class AHTRocketLauncher;
class AHTCannonProjectile;

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRocketLauncherFireDelegate, float, CooldownTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponFireDelegate, int32, WeaponIndex, float, CooldownTime);

UENUM()
enum EAvailableWeapons
{
	Cannon,
	RocketLauncher
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOVERTANKS_API UHTWeaponsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnWeaponFireDelegate OnWeaponFire;

	UHTWeaponsComponent();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void AttemptToShoot(const FHitResult& Hit);
	
	void SwitchToNextWeapon();
	void SwitchToPrevWeapon();

	void SwitchToCannon();
	void SwitchToRockets();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDestroyAttachedWeapons();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AHTCannonProjectile> CannonProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AHTRocketProjectile> RocketProjectileClass;

	UPROPERTY()
	UStaticMeshComponent* TankCannonMesh;
	
	UPROPERTY()
	UStaticMeshComponent* TankBarrelMesh;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAttemptToShoot();

	/**
	 * Main Cannon
	 */
	bool bIsMainCannonOnCooldown = false;
	FTimerHandle MainCannonCooldownTimerHandle;

	void ClearMainCannonCooldown();
	void SpawnProjectile();

	/**
	 * Rocket Launcher
	 */
	void CreateAndAttachRocketLauncher();

	UPROPERTY()
	AHTRocketLauncher* RocketLauncher = nullptr;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAttemptToShootRocketLauncher(const FHitResult& Hit);

	/**
	 * Weapon switching
	 */
	EAvailableWeapons CurrentWeapon = EAvailableWeapons::Cannon;


	/**
	 * Rocket Launcher Target
	 */
	FHitResult& ClientRocketLauncherTarget = *(new FHitResult());
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastShowRocketTarget(const FHitResult& Hit);

	void ShowRocketTarget(const FHitResult& Hit) const;

	void ClearRocketTarget();

	UFUNCTION(Client, Unreliable)
	void ClientOnFire(int32 WeaponIndex, float CooldownTime);
};
