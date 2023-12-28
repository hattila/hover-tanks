// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "HoverTanks/HoverTank.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
ARocketProjectile::ARocketProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);
	bAlwaysRelevant = true;

	// set actor lifetime to 5 seconds
	InitialLifeSpan = 5.f;

	// initialize the Sphere Collider, make it the root component
	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collider"));
	RootComponent = SphereCollider;

	// initialize the mesh, add it under the sphere collider
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);

	// initialize the projectile movement component
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = 1500.f;
	ProjectileMovementComponent->MaxSpeed = 40000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->bIsHomingProjectile = false;
	ProjectileMovementComponent->HomingAccelerationMagnitude = 20000.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset(TEXT("/Game/HoverTanks/Actors/Projectiles/Rocket"));
	UStaticMesh* ProjectileMeshObject = ProjectileMeshAsset.Object;
	ProjectileMesh->SetStaticMesh(ProjectileMeshObject);

	// CollisionProfile.Name = "Projectile"
	SphereCollider->SetCollisionProfileName(TEXT("Projectile"), true);

	ProjectileMesh->SetCollisionProfileName(TEXT("NoCollision"));
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set the mesh transform scale to .5
	ProjectileMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));

	/**
	 * Material
	 */
	// find and initialize the material instance: MI_EmissiveCannonProjectile
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> EmissiveCannonProjectileMaterialAsset(TEXT("/Game/HoverTanks/Materials/MI_EmissiveCannonProjectile"));
	UMaterialInstance* EmissiveCannonProjectileMaterialObject = EmissiveCannonProjectileMaterialAsset.Object;
	ProjectileMesh->SetMaterial(0, EmissiveCannonProjectileMaterialObject);

	/**
	 * FX
	 */
	SmokeTrailFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Smoke Trail FX"));
	SmokeTrailFX->SetupAttachment(RootComponent);
	SmokeTrailFX->SetAutoActivate(true);

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> SmokeTrailEmitterAsset(TEXT("/Game/HoverTanks/Niagara/NS_SmokeTrail"));
	UNiagaraSystem* SmokeTrailEmitterObject = SmokeTrailEmitterAsset.Object;
	SmokeTrailFX->SetAsset(SmokeTrailEmitterObject);

	// setup the explosion FX
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> ExplosionEmitterAsset(TEXT("/Game/HoverTanks/Niagara/NS_Explosion"));
	UNiagaraSystem* ExplosionEmitterObject = ExplosionEmitterAsset.Object;
	ExplosionFX = ExplosionEmitterObject;
	
}

void ARocketProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARocketProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SphereCollider->OnComponentHit.AddDynamic(this, &ARocketProjectile::OnHit);
		SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &ARocketProjectile::OnOverlap);

		const FTimerDelegate IncreaseSpeedTimerDelegate = FTimerDelegate::CreateUObject(
			this,
			&ARocketProjectile::SetHomingTargetDelayed
		);

		GetWorld()->GetTimerManager().SetTimer(DelayedHomingTargetTimerHandle, IncreaseSpeedTimerDelegate,1,false, 1.f);
	}
}

void ARocketProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	MulticastSpawnExplosionFX(Hit.Location, Hit.ImpactNormal.Rotation());
	
	DelayedDestroy();	
}

void ARocketProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	AActor* MyOwner = GetOwner();
	if (MyOwner == nullptr)
	{
		DelayedDestroy();
		return;
	}

	if (OtherActor && OtherActor != this && OtherActor != MyOwner)
	{
		// Create the Niagara System Explosion FX in at the hit location
		MulticastSpawnExplosionFX(Hit.Location, Hit.ImpactNormal.Rotation());
		
		// apply damage to the OtherActor
		UGameplayStatics::ApplyDamage(
			OtherActor,
			Damage,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass()
		);

		DrawDebugSphere(GetWorld(), Hit.Location, 25.f, 12, FColor::Purple, false, 5.f, 0, 1.f);
		DelayedDestroy();
	}
}

void ARocketProjectile::DelayedDestroy()
{
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle,this, &ARocketProjectile::DoDestroy,3,false, 3.f);
	MulticastDeactivateRocket();	
}

void ARocketProjectile::DoDestroy()
{
	GetWorld()->GetTimerManager().ClearTimer(DestroyTimerHandle);
	Destroy();
}

void ARocketProjectile::SetHomingTargetDelayed()
{
	if (!RocketTargetHitResult.IsValidBlockingHit())
	{
		return;
	}

	ProjectileMovementComponent->bIsHomingProjectile = true;

	// should be targetable interface
	if (AHoverTank* HoverTank = Cast<AHoverTank>(RocketTargetHitResult.GetActor()))
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

void ARocketProjectile::MulticastDeactivateRocket_Implementation()
{
	// turn off collision
	SphereCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	// hide the mesh
	ProjectileMesh->SetVisibility(false, true);

	// turn off the emitter
	SmokeTrailFX->Deactivate();
}

void ARocketProjectile::MulticastSpawnExplosionFX_Implementation(FVector Location, FRotator Rotation)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ExplosionFX,
		Location,
		Rotation
	);
}