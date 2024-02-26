// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HTProjectile.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "HTRocketProjectile.generated.h"

class UGameplayEffect;
class UProjectileMovementComponent;

UCLASS()
class HOVERTANKS_API AHTRocketProjectile : public AHTProjectile
{
	GENERATED_BODY()

public:
	AHTRocketProjectile();

	void SetRocketTargetHitResult(const FHitResult& Hit) { RocketTargetHitResult = Hit; }
	
protected:
	virtual void BeginPlay() override;

	virtual void DelayedDestroy() override;
	virtual void DoDestroy() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Class Specifics|Components", meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* SmokeTrailFX = nullptr;

private:
	FTimerHandle DestroyTimerHandle;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDeactivateRocket();

	FHitResult RocketTargetHitResult;

	void SetHomingTargetDelayed();
};
