// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponsComponent.generated.h"

class ARocketLauncher;
class ATankProjectile;

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
	void AttemptToShoot();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ATankProjectile> ProjectileClass;

	UPROPERTY()
	UStaticMeshComponent* TankCannonMesh;
	
	UPROPERTY()
	UStaticMeshComponent* TankBarrelMesh;

	bool bIsMainCannonOnCooldown = false;
	FTimerHandle MainCannonCooldownTimerHandle;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAttemptToShoot();

	void ClearMainCannonCooldown();
	void SpawnProjectile();


	void CreateAndAttachRocketLauncher();
	ARocketLauncher* RocketLauncher = nullptr;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAttemptToShootRocketLauncher();
};
