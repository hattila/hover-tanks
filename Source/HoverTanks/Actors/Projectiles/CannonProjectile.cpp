// Fill out your copyright notice in the Description page of Project Settings.


#include "CannonProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACannonProjectile::ACannonProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// set actor lifetime to 3 seconds
	InitialLifeSpan = 3.f;

	// initialize the Sphere Collider, make it the root component
	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collider"));
	RootComponent = SphereCollider;

	// initialize the mesh, add it under the sphere collider
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);

	// initialize the projectile movement component
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->InitialSpeed = 20000.f;
	ProjectileMovementComponent->MaxSpeed = 40000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	UStaticMesh* ProjectileMeshObject = ProjectileMeshAsset.Object;
	ProjectileMesh->SetStaticMesh(ProjectileMeshObject);

	// CollisionProfile.Name = "Projectile"
	SphereCollider->SetCollisionProfileName(TEXT("Projectile"), true);

	ProjectileMesh->SetCollisionProfileName(TEXT("NoCollision"));
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set the mesh transform scale to .5
	ProjectileMesh->SetWorldScale3D(FVector(.5f, .5f, .5f));

	/**
	 * Material
	 */
	// find and initialize the material instance: MI_EmissiveCannonProjectile
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> EmissiveCannonProjectileMaterialAsset(TEXT("/Game/HoverTanks/Materials/MI_EmissiveCannonProjectile"));
	UMaterialInstance* EmissiveCannonProjectileMaterialObject = EmissiveCannonProjectileMaterialAsset.Object;
	ProjectileMesh->SetMaterial(0, EmissiveCannonProjectileMaterialObject);
}

// Called when the game starts or when spawned
void ACannonProjectile::BeginPlay()
{
	Super::BeginPlay();

	// bind the OnHit event to the OnHit function
	if (HasAuthority())
	{
		SphereCollider->OnComponentHit.AddDynamic(this, &ACannonProjectile::OnHit);
		SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &ACannonProjectile::OnOverlap);
	}
}

// Called every frame
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
	DrawDebugSphere(GetWorld(), Hit.Location, 25.f, 12, FColor::Red, false, 5.f, 0, 1.f);
	
	BounceCount++;
	
	if (BounceCount > 1)
	{
		Destroy();
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
		Destroy();
		return;
	}

	if (OtherActor && OtherActor != this && OtherActor != MyOwner)
	{
		// apply damage to the OtherActor
		UGameplayStatics::ApplyDamage(
			OtherActor,
			50.f,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass()
		);

		DrawDebugSphere(GetWorld(), Hit.Location, 25.f, 12, FColor::Purple, false, 5.f, 0, 1.f);
		Destroy();
	}
}