// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketProjectile.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "HoverTanks/Pawns/HoverTank.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
ARocketProjectile::ARocketProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(true);
	bAlwaysRelevant = true;

	InitialLifeSpan = 5.f;

	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collider"));
	RootComponent = SphereCollider;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);

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

	ProjectileMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));

	// GAS
	static ConstructorHelpers::FClassFinder<UGameplayEffect> DamageEffectAsset(TEXT("/Game/HoverTanks/GAS/GE_Damage_GenericSetMagnitude"));
	DamageEffect = DamageEffectAsset.Class;

	static ConstructorHelpers::FClassFinder<UGameplayEffect> RecentlyDamagedEffectAsset(TEXT("/Game/HoverTanks/GAS/GE_Damage_RecentlyDamaged"));
	RecentlyDamagedEffect = RecentlyDamagedEffectAsset.Class;
	
	/**
	 * Material
	 */
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

void ARocketProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
		MulticastSpawnExplosionFX(Hit.Location, Hit.ImpactNormal.Rotation());

		IAbilitySystemInterface* ActorWithAbilitySystem = Cast<IAbilitySystemInterface>(OtherActor);
		if (DamageEffect != nullptr && ActorWithAbilitySystem != nullptr && RecentlyDamagedEffect != nullptr)
		{
			UAbilitySystemComponent* TargetASC = ActorWithAbilitySystem->GetAbilitySystemComponent();
			
			if (TargetASC != nullptr)
			{
				FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
				EffectContext.AddSourceObject(this);
				EffectContext.AddInstigator(GetInstigatorController(), this);
				EffectContext.AddHitResult(Hit);
				
				FGameplayEffectSpecHandle DamageEffectSpecHandle = TargetASC->MakeOutgoingSpec(DamageEffect, 1.f, EffectContext);
				FGameplayEffectSpec* DamageEffectSpec = DamageEffectSpecHandle.Data.Get();
				
				DamageEffectSpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
				if (DamageEffectSpecHandle.IsValid())
				{
					TargetASC->ApplyGameplayEffectSpecToTarget(*DamageEffectSpec, TargetASC);
				}

				TargetASC->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(RecentlyDamagedEffect->GetDefaultObject()), 1.0f, TargetASC->MakeEffectContext());	
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

	// should be a targetable interface
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
	SphereCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	ProjectileMesh->SetVisibility(false, true);

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