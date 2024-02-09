// Fill out your copyright notice in the Description page of Project Settings.


#include "CannonProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACannonProjectile::ACannonProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(true);
	bAlwaysRelevant = true;

	InitialLifeSpan = 3.f;

	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collider"));
	RootComponent = SphereCollider;

	SphereCollider->SetCollisionProfileName(TEXT("Projectile"), true);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	UStaticMesh* ProjectileMeshObject = ProjectileMeshAsset.Object;
	ProjectileMesh->SetStaticMesh(ProjectileMeshObject);
	ProjectileMesh->SetCollisionProfileName(TEXT("NoCollision"));
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetWorldScale3D(FVector(.5f, .5f, .5f));
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->InitialSpeed = 20000.f;
	ProjectileMovementComponent->MaxSpeed = 40000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true;

	// GAS
	// find and initialize the gameplay effect: GE_Damage_CannonProjectile
	static ConstructorHelpers::FClassFinder<UGameplayEffect> DamageEffectAsset(TEXT("/Game/HoverTanks/GAS/GE_Damage_CannonProjectile"));
	DamageEffect = DamageEffectAsset.Class;
	
	/**
	 * Material
	 */
	// find and initialize the material instance: MI_EmissiveCannonProjectile
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> EmissiveCannonProjectileMaterialAsset(TEXT("/Game/HoverTanks/Materials/MI_EmissiveCannonProjectile"));
	UMaterialInstance* EmissiveCannonProjectileMaterialObject = EmissiveCannonProjectileMaterialAsset.Object;
	ProjectileMesh->SetMaterial(0, EmissiveCannonProjectileMaterialObject);

	// setup the explosion FX
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> ExplosionEmitterAsset(TEXT("/Game/HoverTanks/Niagara/NS_Explosion"));
	UNiagaraSystem* ExplosionEmitterObject = ExplosionEmitterAsset.Object;
	ExplosionFX = ExplosionEmitterObject;
}

// Called when the game starts or when spawned
void ACannonProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SphereCollider->OnComponentHit.AddDynamic(this, &ACannonProjectile::OnHit);
		SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &ACannonProjectile::OnOverlap);
	}
}

void ACannonProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACannonProjectile::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	// DrawDebugSphere(GetWorld(), Hit.Location, 25.f, 12, FColor::Red, false, 5.f, 0, 1.f);
	
	BounceCount++;
	
	if (BounceCount > MaxBounceCount)
	{
		MulticastSpawnExplosionFX(Hit.Location, Hit.ImpactNormal.Rotation());
		DelayedDestroy();
	}
}

void ACannonProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool FromSweep,
	const FHitResult& Hit
)
{
	AActor* MyOwner = GetOwner();
	if (MyOwner == nullptr)
	{
		DelayedDestroy();
		return;
	}

	if (OtherActor && OtherActor != this && OtherActor != MyOwner)
	{
		MulticastSpawnExplosionFX(Hit.Location, Hit.ImpactNormal.Rotation());

		IAbilitySystemInterface* ActorWithAbilitySystem = Cast<IAbilitySystemInterface>(OtherActor);
		if (DamageEffect != nullptr && ActorWithAbilitySystem != nullptr)
		{
			UAbilitySystemComponent* AbilitySystemComponent = ActorWithAbilitySystem->GetAbilitySystemComponent();
			
			if (AbilitySystemComponent != nullptr)
			{
				FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
				EffectContext.AddSourceObject(this);
				EffectContext.AddHitResult(Hit);
				
				FGameplayEffectSpecHandle DamageEffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageEffect, 1.f, EffectContext);
				if (DamageEffectSpecHandle.IsValid())
				{
					AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), AbilitySystemComponent);
				}
			}
		}
		else
		{
			UGameplayStatics::ApplyDamage(
				OtherActor,
				Damage,
				GetInstigatorController(),
				this,
				UDamageType::StaticClass()
			);	
		}

		// DrawDebugSphere(GetWorld(), Hit.Location, 25.f, 12, FColor::Purple, false, 5.f, 0, 1.f);
		DelayedDestroy();
	}
}

void ACannonProjectile::DelayedDestroy()
{
	FTimerHandle DestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle,this, &ACannonProjectile::DoDestroy,1,false, 1.f);

	MulticastDeactivateProjectile();
}

void ACannonProjectile::DoDestroy()
{
	Destroy();
}

void ACannonProjectile::MulticastDeactivateProjectile_Implementation()
{
	SphereCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	ProjectileMesh->SetVisibility(false, true);
}

void ACannonProjectile::MulticastSpawnExplosionFX_Implementation(FVector Location, FRotator Rotation)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ExplosionFX,
		Location,
		Rotation
	);
}
