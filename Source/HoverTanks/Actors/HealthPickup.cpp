// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"

#include "HoverTanks/HoverTank.h"
#include "HoverTanks/Components/HealthComponent.h"
#include "Net/UnrealNetwork.h"


AHealthPickup::AHealthPickup()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// initialize the box collider
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxCollider->SetCollisionProfileName(TEXT("Pickup"));
	
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
	PickupMesh->SetWorldScale3D(FVector(1.5, 1.5, 1.5));

	// find the material instance MI_EmissiveHealthPickup
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> EmissiveHealthPickupMaterialAsset(TEXT("/Game/HoverTanks/Materials/MI_EmissiveHealthPickup"));
	UMaterialInstance* EmissiveHealthPickupMaterialObject = EmissiveHealthPickupMaterialAsset.Object;
	PickupMesh->SetMaterial(0, EmissiveHealthPickupMaterialObject);
}

void AHealthPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// replicate bIsPickedUp
	DOREPLIFETIME(AHealthPickup, bIsPickedUp);
}

void AHealthPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsSpawningIn)
	{
		SpawnInAnimation(DeltaTime);
	}
	
	if (!bIsPickedUp && !bIsSpawningIn)
	{
		FloatingAnimation(DeltaTime);
	}

	if (bIsPickedUp)
	{
		DeSpawnAnimation(DeltaTime);
	}
	
}

void AHealthPickup::BeginPlay()
{
	Super::BeginPlay();

	InitialZ = PickupMesh->GetComponentLocation().Z;

	if (HasAuthority())
	{
		BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &AHealthPickup::OnOverlapBegin);

		bIsSpawningIn = true;
		PickupMesh->SetWorldScale3D(FVector(0.1f, 0.1f, 0.1f));

		FVector Height = PickupMesh->GetComponentLocation();
		Height.Z = InitialZ + 400.0f;
		PickupMesh->SetWorldLocation(Height);

		FTimerHandle SpawnInTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(SpawnInTimerHandle, this, &AHealthPickup::SpawnInComplete, 1.0f, false);
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

			BoxCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			bIsPickedUp = true;
			DelayedDestroy();
		}
	}
}

void AHealthPickup::DelayedDestroy()
{
	FTimerHandle DestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AHealthPickup::DoDestroy, 1.0f, false);
}

void AHealthPickup::DoDestroy()
{
	Destroy();
}

void AHealthPickup::SpawnInComplete()
{
	bIsSpawningIn = false;
}

void AHealthPickup::SpawnInAnimation(const float DeltaTime) const
{
	FVector NewScale = PickupMesh->GetComponentScale();
	NewScale.X = FMath::Lerp(NewScale.X, 1.0f, DeltaTime * 2);
	NewScale.Y = FMath::Lerp(NewScale.Y, 1.0f, DeltaTime * 2);
	NewScale.Z = FMath::Lerp(NewScale.Z, 1.0f, DeltaTime * 2);
	PickupMesh->SetWorldScale3D(NewScale);

	FVector NewHeight = PickupMesh->GetComponentLocation();
	NewHeight.Z = FMath::Lerp(NewHeight.Z, InitialZ, DeltaTime * 2);
	PickupMesh->SetWorldLocation(NewHeight);
}

void AHealthPickup::FloatingAnimation(const float DeltaTime) const
{
	FVector NewLocation = PickupMesh->GetComponentLocation();
	NewLocation.Z = InitialZ + FMath::Sin(GetGameTimeSinceCreation() * 2) * 50.0f; // Adjust the amplitude as needed
	PickupMesh->SetWorldLocation(NewLocation);

	// rotate the mesh constantly
	FRotator NewRotation = PickupMesh->GetComponentRotation();
	NewRotation.Yaw += DeltaTime * 100.0f;
	PickupMesh->SetWorldRotation(NewRotation);	
}

void AHealthPickup::DeSpawnAnimation(const float DeltaTime) const
{
	FVector NewScale = PickupMesh->GetComponentScale();
	NewScale.X = FMath::Lerp(NewScale.X, .05f, DeltaTime * 2);
	NewScale.Y = FMath::Lerp(NewScale.Y, .05f, DeltaTime * 2);
	NewScale.Z = FMath::Lerp(NewScale.Z, .05f, DeltaTime * 2);
	PickupMesh->SetWorldScale3D(NewScale);

	FVector NewHeight = PickupMesh->GetComponentLocation();
	NewHeight.Z = FMath::Lerp(NewHeight.Z, InitialZ + 400.0f, DeltaTime * 3);
	PickupMesh->SetWorldLocation(NewHeight);
}


