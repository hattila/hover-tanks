// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RocketLauncher.generated.h"

class ARocketProjectile;

UCLASS()
class HOVERTANKS_API ARocketLauncher : public AActor
{
	GENERATED_BODY()

public:
	ARocketLauncher();
	virtual void Tick(float DeltaTime) override;

	void SetRocketTargetHitResult(const FHitResult& Hit) const { RocketTargetHitResult = Hit; }
	void Fire();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UStaticMeshComponent* BaseMesh;

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
	USceneComponent* RocketTargetLocationComponent = nullptr;
};
