// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HTProjectile.h"
#include "GameFramework/Actor.h"
#include "HTCannonProjectile.generated.h"

class UGameplayEffect;
class UNiagaraSystem;
class UProjectileMovementComponent;

UCLASS()
class HOVERTANKS_API AHTCannonProjectile : public AHTProjectile
{
	GENERATED_BODY()
	
public:	
	AHTCannonProjectile();

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class Specifics|Movement", meta = (AllowPrivateAccess = "true"))
	float MaxBounceCount = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Class Specifics|Movement", meta = (AllowPrivateAccess = "true"))
	float CurrentBounceCount = 0;
};
