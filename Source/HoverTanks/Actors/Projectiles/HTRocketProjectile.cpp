// Fill out your copyright notice in the Description page of Project Settings.


#include "HTRocketProjectile.h"

#include "HoverTanks/Pawns/HoverTank/HTHoverTank.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AHTRocketProjectile::AHTRocketProjectile()
{
	InitialLifeSpan = 5.f;
	Damage = 40.f;

	ProjectileMovementComponent->InitialSpeed = 1500.f;
	ProjectileMovementComponent->MaxSpeed = 40000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->bIsHomingProjectile = false;
	ProjectileMovementComponent->HomingAccelerationMagnitude = 20000.f;
	
	/**
	 * FX
	 */
	SmokeTrailFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Smoke Trail FX"));
	SmokeTrailFX->SetupAttachment(RootComponent);
	SmokeTrailFX->SetAutoActivate(true);
}

void AHTRocketProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		const FTimerDelegate IncreaseSpeedTimerDelegate = FTimerDelegate::CreateUObject(
			this,
			&AHTRocketProjectile::SetHomingTargetDelayed
		);

		FTimerHandle DelayedHomingTargetTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(DelayedHomingTargetTimerHandle, IncreaseSpeedTimerDelegate,1,false, 1.f);
	}
}


void AHTRocketProjectile::DelayedDestroy()
{
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle,this, &AHTRocketProjectile::DoDestroy,3,false, 3.f);
	MulticastDeactivateRocket();	
}

void AHTRocketProjectile::DoDestroy()
{
	GetWorld()->GetTimerManager().ClearTimer(DestroyTimerHandle);
	Super::DoDestroy();
}

void AHTRocketProjectile::SetHomingTargetDelayed()
{
	if (!RocketTargetHitResult.IsValidBlockingHit())
	{
		return;
	}

	ProjectileMovementComponent->bIsHomingProjectile = true;

	// should be a targetable interface
	if (AHTHoverTank* HoverTank = Cast<AHTHoverTank>(RocketTargetHitResult.GetActor()))
	{
		ProjectileMovementComponent->HomingTargetComponent = HoverTank->GetRootComponent();
	}
	else
	{
		USceneComponent* RocketTargetLocationComponent = NewObject<USceneComponent>(this, TEXT("RocketTargetLocationComponent"));
		RocketTargetLocationComponent->SetWorldLocation(RocketTargetHitResult.Location);
		
		ProjectileMovementComponent->HomingTargetComponent = RocketTargetLocationComponent;
	}
}

void AHTRocketProjectile::MulticastDeactivateRocket_Implementation()
{
	SphereCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	ProjectileMesh->SetVisibility(false, true);

	SmokeTrailFX->Deactivate();
}
