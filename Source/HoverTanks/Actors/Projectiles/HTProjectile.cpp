// Fill out your copyright notice in the Description page of Project Settings.


#include "HTProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AHTProjectile::AHTProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(true);
	bAlwaysRelevant = true;

	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collider"));
	RootComponent = SphereCollider;

	SphereCollider->SetCollisionProfileName(TEXT("Projectile"), true);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionProfileName(TEXT("NoCollision"));
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->InitialSpeed = 20000.0; // change on child BPs
	ProjectileMovementComponent->MaxSpeed = 40000.0; // change on child BPs
}

void AHTProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (SphereCollider == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("AHTProjectile::BeginPlay SphereCollider is null"));
			return;
		}
		
		SphereCollider->OnComponentHit.AddDynamic(this, &AHTProjectile::OnHit);
		SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &AHTProjectile::OnOverlap);
	}
}

void AHTProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	MulticastSpawnExplosionFX(Hit.Location, Hit.ImpactNormal.Rotation());
	DelayedDestroy();
}

void AHTProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp,
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

				if (RecentlyDamagedEffect != nullptr)
				{
					TargetASC->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(RecentlyDamagedEffect->GetDefaultObject()), 1.0f, TargetASC->MakeEffectContext());	
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

void AHTProjectile::DelayedDestroy()
{
	FTimerHandle DestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle,this, &AHTProjectile::DoDestroy,1,false, 1.f);

	MulticastDeactivateProjectile();
}

void AHTProjectile::DoDestroy()
{
	Destroy();
}

void AHTProjectile::MulticastDeactivateProjectile_Implementation()
{
	SphereCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	ProjectileMesh->SetVisibility(false, true);
}

void AHTProjectile::MulticastSpawnExplosionFX_Implementation(FVector Location, FRotator Rotation)
{
	if (ExplosionFX == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AHTProjectile::MulticastSpawnExplosionFX_Implementation ExplosionFX is null"));
		return;
	}
	
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ExplosionFX,
		Location,
		Rotation
	);
}



