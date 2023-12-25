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
	
	void Fire();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UStaticMeshComponent* BaseMesh;

	bool bIsOnCooldown = false;
	
	float FireCooldownTime = 3.0f;
	float TimeBetweenShots = 0.5f;

	FTimerHandle FireCooldownTimerHandle;
	FTimerHandle FireTimerHandle;

	void ClearFireCooldownTimer();

	void SpawnProjectile();
};
