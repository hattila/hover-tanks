// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RocketLauncher.generated.h"

UCLASS()
class HOVERTANKS_API ARocketLauncher : public AActor
{
	GENERATED_BODY()

public:
	ARocketLauncher();

	virtual void Tick(float DeltaTime) override;
	
	void SetRocketTargetLocation(const FVector& InLocation) { RocketTargetLocation = InLocation; }
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
	void BurstFire(USceneComponent* InRocketTargetLocationComponent);
	
	void SpawnProjectile(USceneComponent* InRocketTargetLocationComponent);

	FVector RocketTargetLocation = FVector::ZeroVector;
	USceneComponent* RocketTargetLocationComponent = nullptr;
};
