// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponsComponent.generated.h"

class ARocketLauncher;
class ACannonProjectile;

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
	// Sets default values for this component's properties
	UWeaponsComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void AttemptToShoot(const FVector& LocationUnderTheCrosshair);
	
	void SwitchToNextWeapon();
	void SwitchToPrevWeapon();

	void DestroyAttachedWeapons();

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
	void ServerAttemptToShootRocketLauncher(const FVector& LocationUnderTheCrosshair);

	/**
	 * Weapon switching
	 */
	EAvailableWeapons CurrentWeapon = EAvailableWeapons::Cannon;
};
