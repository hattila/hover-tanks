// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawner.h"


// Sets default values
APickupSpawner::APickupSpawner(): BoxCollider(nullptr), BaseMesh(nullptr)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// initialize the box collider
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxCollider->SetCollisionProfileName(TEXT("PickupSpawner"));
	BoxCollider->SetBoxExtent(FVector(100.0f, 100.0f, 200.0f));
	RootComponent = BoxCollider;

	// initialize the baseMesh as the engine cube
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BaseMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	UStaticMesh* BaseMeshObject = BaseMeshAsset.Object;
	BaseMesh->SetStaticMesh(BaseMeshObject);
	
	BaseMesh->SetCollisionProfileName(TEXT("NoCollision"));
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BaseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -240.0f));

	HealthPickupClass = AHealthPickup::StaticClass();
}

// Called every frame
void APickupSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when the game starts or when spawned
void APickupSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnPickup();

		BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &APickupSpawner::OnOverlapBegin);
		BoxCollider->OnComponentEndOverlap.AddDynamic(this, &APickupSpawner::OnOverlapEnd);
	}
	
}

void APickupSpawner::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// if the OtherActor is a HealthPickup
	if (OtherActor->IsA(AHealthPickup::StaticClass()))
	{
		bCurrentlyHasPickup = true;
		// UE_LOG(LogTemp, Warning, TEXT("PickupSpawner (OnOverlapBegin) now has a pickup"));
	}
}

void APickupSpawner::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// if the OtherActor was a HealthPickup
	if (OtherActor->IsA(AHealthPickup::StaticClass()))
	{
		// set bCurrentlyHasPickup to false
		bCurrentlyHasPickup = false;
		// UE_LOG(LogTemp, Warning, TEXT("PickupSpawner (OnOverlapEnd) now does not have a pickup, starting timer"));

		// set a timer to respawn the pickup
		GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &APickupSpawner::SpawnPickup, ItemRespawnTime, false, ItemRespawnTime);
	}

	// UE_LOG(LogTemp, Warning, TEXT("PickupSpawner (OnOverlapEnd) the OtherActor was a %s"), *OtherActor->GetName());
}

void APickupSpawner::SpawnPickup()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = nullptr;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AHealthPickup* SpawnedPickup = GetWorld()->SpawnActor<AHealthPickup>(HealthPickupClass, GetActorLocation(), GetActorRotation(), SpawnParams);

	bCurrentlyHasPickup = true;
	// UE_LOG(LogTemp, Warning, TEXT("PickupSpawner (SpawnPickup) now has a pickup"));
}
