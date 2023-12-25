// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "RocketProjectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class HOVERTANKS_API ARocketProjectile : public AActor
{
	GENERATED_BODY()

public:
	ARocketProjectile();

	virtual void Tick(float DeltaTime) override;

	void SetHomingTarget(USceneComponent* SceneComponent) const { ProjectileMovementComponent->HomingTargetComponent = SceneComponent; }
	void SetIsHoming(const bool bIsHoming) const { ProjectileMovementComponent->bIsHomingProjectile = bIsHoming; }
	void SetProjectileSpeed(const float Speed) const { ProjectileMovementComponent->InitialSpeed = Speed; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereCollider;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FCollisionProfileName CollisionProfile;
	
	// create a static mesh component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovementComponent;
	
};
