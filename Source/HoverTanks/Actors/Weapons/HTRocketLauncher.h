// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HTRocketLauncher.generated.h"

class AHTProjectile;
class AHTRocketProjectile;

UCLASS()
class HOVERTANKS_API AHTRocketLauncher : public AActor
{
	GENERATED_BODY()

public:
	AHTRocketLauncher();

	// define an Init function that takes in a class of type AHTProjectile
	void Init(TSubclassOf<AHTRocketProjectile> InProjectileClass);
	
	virtual void Tick(float DeltaTime) override;

	void SetRocketTargetHitResult(const FHitResult& Hit) const { RocketTargetHitResult = Hit; }
	bool Fire();
	float GetFireCooldownTime() const { return FireCooldownTime; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AHTRocketProjectile> ProjectileClass;

	bool bIsOnCooldown = false;
	
	float FireCooldownTime = 3.0f;
	float TimeBetweenShots = 0.15f;

	FTimerHandle FireCooldownTimerHandle;
	FTimerHandle FireTimerHandle;

	void ClearFireCooldownTimer();

	int MaxBurstFireCount = 3;
	int CurrentFireCount = 0;

	void BurstFire();
	void SpawnProjectile(const FHitResult& InTargetHitResult) const;

	FVector RocketTargetLocation = FVector::ZeroVector;
	
	FHitResult& RocketTargetHitResult = *(new FHitResult());

	UPROPERTY()
	USceneComponent* RocketTargetLocationComponent = nullptr;
};
