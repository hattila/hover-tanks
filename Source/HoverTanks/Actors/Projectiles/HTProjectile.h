// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HTProjectile.generated.h"

class UGameplayEffect;
class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS()
class HOVERTANKS_API AHTProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHTProjectile();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Class Specifics|Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* SphereCollider = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Class Specifics|Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ProjectileMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Class Specifics|Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovementComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class Specifics|Components", meta=(AllowPrivateAccess="true"))
	UNiagaraSystem* ExplosionFX = nullptr;

	/**
	 * GAS
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class Specifics|Components|GAS", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class Specifics|Components|GAS", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> RecentlyDamagedEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class Specifics|Property", meta = (AllowPrivateAccess = "true"))
	float Damage = 0;
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool FromSweep, const FHitResult& Hit);

	virtual void DelayedDestroy();
	virtual void DoDestroy();

	UFUNCTION(NetMulticast, Unreliable)
	virtual void MulticastDeactivateProjectile();

	UFUNCTION(NetMulticast, Unreliable)
	virtual void MulticastSpawnExplosionFX(FVector Location, FRotator Rotation);
	
};
