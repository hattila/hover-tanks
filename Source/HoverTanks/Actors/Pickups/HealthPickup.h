// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "HealthPickup.generated.h"

UCLASS()
class HOVERTANKS_API AHealthPickup : public AActor
{
	GENERATED_BODY()

public:
	AHealthPickup();
	// replicateion
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* BoxCollider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PickupMesh;

	float InitialZ;

	UPROPERTY(Replicated)
	bool bIsPickedUp = false;

	void DelayedDestroy();
	void DoDestroy();

	UPROPERTY(Replicated)
	bool bIsSpawningIn = false;
	void SpawnInComplete();

	void SpawnInAnimation(float DeltaTime) const;
	void FloatingAnimation(float DeltaTime) const;
	void DeSpawnAnimation(float DeltaTime) const;
};
