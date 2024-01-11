// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HealthPickup.h"
#include "GameFramework/Actor.h"
#include "PickupSpawner.generated.h"

class UBoxComponent;

UCLASS()
class HOVERTANKS_API APickupSpawner : public AActor
{
	GENERATED_BODY()

public:
	APickupSpawner();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* BoxCollider = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BaseMesh = nullptr;
	
	TSubclassOf<AHealthPickup> HealthPickupClass;

	bool bCurrentlyHasPickup = false;
	
	FTimerHandle RespawnTimerHandle;

	float ItemRespawnTime = 10.0f;
	
	void SpawnPickup();

	
};