// Fill out your copyright notice in the Description page of Project Settings.

#include "HTPickupSpawner.h"

AHTPickupSpawner::AHTPickupSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// initialize the box collider
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxCollider->SetCollisionProfileName(TEXT("PickupSpawner"));
	BoxCollider->SetBoxExtent(FVector(100.0f, 100.0f, 200.0f));
	RootComponent = BoxCollider;

	// initialize the baseMesh
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);

	/**
	 * Example of how to reference a static mesh object from the editor, and set it
	 */
	// static ConstructorHelpers::FObjectFinder<UStaticMesh> BaseMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	// static ConstructorHelpers::FObjectFinder<UStaticMesh> BaseMeshAsset(TEXT("/Game/HoverTanks/Actors/Pickups/PickupSpawner"));
	// UStaticMesh* BaseMeshObject = BaseMeshAsset.Object;
	// BaseMesh->SetStaticMesh(BaseMeshObject);
	
	BaseMesh->SetCollisionProfileName(TEXT("NoCollision"));
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BaseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -192.0f));

	/**
	 * Example of how to reference a material object from the editor, and set it
	 */
	// static ConstructorHelpers::FObjectFinder<UMaterialInterface> TankBaseMaterialAsset(TEXT("/Game/Megascans/surfaces/Painted_Gun_Metal_shrbehqc/MI_Painted_Gun_Metal_shrbehqc_4K"));
	// UMaterialInterface* TankBaseMaterialAssetObject = TankBaseMaterialAsset.Object;
	// BaseMesh->SetMaterial(0, TankBaseMaterialAssetObject);

	EffectPickupClass = AHTGameplayEffectPickup::StaticClass();
	
}

void AHTPickupSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnPickup();

		BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &AHTPickupSpawner::OnOverlapBegin);
		BoxCollider->OnComponentEndOverlap.AddDynamic(this, &AHTPickupSpawner::OnOverlapEnd);
	}
	
}

void AHTPickupSpawner::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(AHTGameplayEffectPickup::StaticClass()))
	{
		bCurrentlyHasPickup = true;
	}
}

void AHTPickupSpawner::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA(AHTGameplayEffectPickup::StaticClass()))
	{
		bCurrentlyHasPickup = false;

		GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AHTPickupSpawner::SpawnPickup, ItemRespawnTime, false, ItemRespawnTime);
	}
}

void AHTPickupSpawner::SpawnPickup()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = nullptr;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AHTGameplayEffectPickup* SpawnedPickup = GetWorld()->SpawnActor<AHTGameplayEffectPickup>(EffectPickupClass, GetActorLocation(), GetActorRotation(), SpawnParams);

	bCurrentlyHasPickup = true;
}
