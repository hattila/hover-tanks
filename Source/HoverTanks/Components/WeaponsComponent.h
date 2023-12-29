// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponsComponent.generated.h"

class ARocketLauncher;
class ACannonProjectile;

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRocketLauncherFireDelegate, float, CooldownTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponFireDelegate, int32, WeaponIndex, float, CooldownTime);

UENUM()
enum EAvailableWeapons
{
	Cannon,
	RocketLauncher
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOVERTANKS_API UWeaponsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnWeaponFireDelegate OnWeaponFire;

	UWeaponsComponent();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	// void AttemptToShoot(const FVector& LocationUnderTheCrosshair);
	void AttemptToShoot(const FHitResult& Hit);
	
	void SwitchToNextWeapon();
	void SwitchToPrevWeapon();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDestroyAttachedWeapons();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ACannonProjectile> ProjectileClass;

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
	ARocketLauncher* RocketLauncher = nullptr;

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

	// FTimerHandle RocketTargetTimerHandle;
	void ClearRocketTarget();

	UFUNCTION(Client, Unreliable)
	void ClientOnFire(int32 WeaponIndex, float CooldownTime);
};
