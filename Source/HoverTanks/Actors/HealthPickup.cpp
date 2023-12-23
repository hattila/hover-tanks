// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"

#include "HoverTanks/HoverTank.h"
#include "HoverTanks/Components/HealthComponent.h"


AHealthPickup::AHealthPickup()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// initialize the box collider
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	// todo: preset: pickup
	// BoxCollider->SetCollisionObjectType(ECC_WorldDynamic);
	BoxCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	BoxCollider->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	RootComponent = BoxCollider;

	// initialize the mesh
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	PickupMesh->SetupAttachment(RootComponent);

	PickupMesh->SetCollisionProfileName(TEXT("NoCollision"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	UStaticMesh* ProjectileMeshObject = MeshAsset.Object;
	PickupMesh->SetStaticMesh(ProjectileMeshObject);

	// Set the mesh transform scale to .5
	PickupMesh->SetWorldScale3D(FVector(1, 1, 1));
	
}

void AHealthPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector NewLocation = PickupMesh->GetComponentLocation();
	NewLocation.Z = InitialZ + FMath::Sin(GetGameTimeSinceCreation() * 2) * 50.0f; // Adjust the amplitude as needed
	PickupMesh->SetWorldLocation(NewLocation);

	// rotate the mesh constantly
	FRotator NewRotation = PickupMesh->GetComponentRotation();
	NewRotation.Yaw += DeltaTime * 100.0f;
	PickupMesh->SetWorldRotation(NewRotation);
}

void AHealthPickup::BeginPlay()
{
	Super::BeginPlay();

	InitialZ = PickupMesh->GetComponentLocation().Z;

	if (HasAuthority())
	{
		BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &AHealthPickup::OnOverlapBegin);
	}
}

void AHealthPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		// todo: canUsePickup interface
		AHoverTank* HoverTank = Cast<AHoverTank>(OtherActor);
		if (HoverTank)
		{
			HoverTank->GetHealthComponent()->Heal(60);
			Destroy();
		}
	}
}


