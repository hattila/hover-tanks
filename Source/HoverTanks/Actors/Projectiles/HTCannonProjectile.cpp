// Fill out your copyright notice in the Description page of Project Settings.

#include "HTCannonProjectile.h"

AHTCannonProjectile::AHTCannonProjectile()
{
	Damage = 60;
}

void AHTCannonProjectile::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	// DrawDebugSphere(GetWorld(), Hit.Location, 25.f, 12, FColor::Red, false, 5.f, 0, 1.f);
	CurrentBounceCount++;
	
	if (CurrentBounceCount > MaxBounceCount)
	{
		Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
	}
}
