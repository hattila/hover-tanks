// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "HTGameplayEffectPickup.generated.h"

class UGameplayEffect;

UCLASS()
class HOVERTANKS_API AHTGameplayEffectPickup : public AActor
{
	GENERATED_BODY()

public:
	AHTGameplayEffectPickup();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	float EffectMagnitude = 60;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* BoxCollider;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|GAS", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> GameplayEffect;
	
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
