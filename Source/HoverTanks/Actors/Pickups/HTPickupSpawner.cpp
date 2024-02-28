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

	// initialize the baseMesh as the engine cube
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);

	// static ConstructorHelpers::FObjectFinder<UStaticMesh> BaseMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BaseMeshAsset(TEXT("/Game/HoverTanks/Actors/Pickups/PickupSpawner"));
	UStaticMesh* BaseMeshObject = BaseMeshAsset.Object;
	BaseMesh->SetStaticMesh(BaseMeshObject);
	
	BaseMesh->SetCollisionProfileName(TEXT("NoCollision"));
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BaseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -192.0f));

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TankBaseMaterialAsset(TEXT("/Game/Megascans/surfaces/Painted_Gun_Metal_shrbehqc/MI_Painted_Gun_Metal_shrbehqc_4K"));
	UMaterialInterface* TankBaseMaterialAssetObject = TankBaseMaterialAsset.Object;
	BaseMesh->SetMaterial(0, TankBaseMaterialAssetObject);

	HealthPickupClass = AHTHealthPickup::StaticClass();
	
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
	if (OtherActor->IsA(AHTHealthPickup::StaticClass()))
	{
		bCurrentlyHasPickup = true;
		// UE_LOG(LogTemp, Warning, TEXT("PickupSpawner (OnOverlapBegin) now has a pickup"));
	}
}

void AHTPickupSpawner::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA(AHTHealthPickup::StaticClass()))
	{
		bCurrentlyHasPickup = false;
		// UE_LOG(LogTemp, Warning, TEXT("PickupSpawner (OnOverlapEnd) now does not have a pickup, starting timer"));

		GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AHTPickupSpawner::SpawnPickup, ItemRespawnTime, false, ItemRespawnTime);
	}

	// UE_LOG(LogTemp, Warning, TEXT("PickupSpawner (OnOverlapEnd) the OtherActor was a %s"), *OtherActor->GetName());
}

void AHTPickupSpawner::SpawnPickup()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = nullptr;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AHTHealthPickup* SpawnedPickup = GetWorld()->SpawnActor<AHTHealthPickup>(HealthPickupClass, GetActorLocation(), GetActorRotation(), SpawnParams);

	bCurrentlyHasPickup = true;
	// UE_LOG(LogTemp, Warning, TEXT("PickupSpawner (SpawnPickup) now has a pickup"));
}
