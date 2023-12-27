// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "RocketProjectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class HOVERTANKS_API ARocketProjectile : public AActor
{
	GENERATED_BODY()

public:
	ARocketProjectile();

	virtual void Tick(float DeltaTime) override;

	void SetRocketTargetLocationComponent(USceneComponent* InRocketTargetLocationComponent) { RocketTargetLocationComponent = InRocketTargetLocationComponent; }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	float Damage = 40;
	
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

private:
	/**
	 * Base Projectile
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereCollider;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FCollisionProfileName CollisionProfile;
	
	// create a static mesh component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovementComponent;

	/**
	 * Rocket spec
	 */

	UNiagaraComponent* SmokeTrailFX = nullptr;

	FTimerHandle DestroyTimerHandle;
	bool bIsDestroyed = false;
	void DelayedDestroy();
	void DoDestroy();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDeactivateRocket();

	
	USceneComponent* RocketTargetLocationComponent = nullptr;

	FTimerHandle DelayedHomingTargetTimerHandle;
	void SetHomingTargetDelayed();
};
