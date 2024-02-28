// Fill out your copyright notice in the Description page of Project Settings.


#include "HTHealthPickup.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "HoverTanks/Pawns/HoverTank.h"

#include "Net/UnrealNetwork.h"

AHTHealthPickup::AHTHealthPickup()
{
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
	PickupMesh->SetWorldScale3D(FVector(1.5, 1.5, 1.5));

	// find the material instance MI_EmissiveHealthPickup
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> EmissiveHealthPickupMaterialAsset(TEXT("/Game/HoverTanks/Materials/MI_EmissiveHealthPickup"));
	UMaterialInstance* EmissiveHealthPickupMaterialObject = EmissiveHealthPickupMaterialAsset.Object;
	PickupMesh->SetMaterial(0, EmissiveHealthPickupMaterialObject);

	// initialize the heal effect
	static ConstructorHelpers::FClassFinder<UGameplayEffect> HealEffectAsset(TEXT("/Game/HoverTanks/Actors/Pickups/GE_HealEffect_Instant"));
	HealEffect = HealEffectAsset.Class;
}

void AHTHealthPickup::BeginPlay()
{
	Super::BeginPlay();

	InitialZ = PickupMesh->GetComponentLocation().Z;

	if (HasAuthority())
	{
		BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &AHTHealthPickup::OnOverlapBegin);

		bIsSpawningIn = true;

		FVector Height = PickupMesh->GetComponentLocation();
		Height.Z = InitialZ + 400.0f;
		PickupMesh->SetWorldLocation(Height);

		FTimerHandle SpawnInTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(SpawnInTimerHandle, this, &AHTHealthPickup::SpawnInComplete, 1.0f, false);
	}
}

void AHTHealthPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHTHealthPickup, bIsPickedUp);
	DOREPLIFETIME(AHTHealthPickup, bIsSpawningIn);
}

void AHTHealthPickup::Tick(float DeltaTime)
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

void AHTHealthPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		bool bVasValidPickup = false;

		IAbilitySystemInterface* ActorWithAbilitySystem = Cast<IAbilitySystemInterface>(OtherActor);
		if (HealEffect != nullptr && ActorWithAbilitySystem != nullptr)
		{
			UAbilitySystemComponent* AbilitySystemComponent = ActorWithAbilitySystem->GetAbilitySystemComponent();
			
			if (AbilitySystemComponent != nullptr)
			{
				FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
				EffectContext.AddSourceObject(this);
				EffectContext.AddInstigator(GetInstigatorController(), this);
				
				FGameplayEffectSpecHandle HealEffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(HealEffect, 1.f, EffectContext);
				FGameplayEffectSpec* HealEffectSpec = HealEffectSpecHandle.Data.Get();
				
				HealEffectSpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Healing")), HealAmount);
				if (HealEffectSpecHandle.IsValid())
				{
					AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*HealEffectSpec, AbilitySystemComponent);
				}
				bVasValidPickup = true;
			}
		}

		if (bVasValidPickup)
		{
			BoxCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			bIsPickedUp = true;
			DelayedDestroy();	
		}
	}
}

void AHTHealthPickup::DelayedDestroy()
{
	FTimerHandle DestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AHTHealthPickup::DoDestroy, 1.0f, false);
}

void AHTHealthPickup::DoDestroy()
{
	Destroy();
}

void AHTHealthPickup::SpawnInComplete()
{
	bIsSpawningIn = false;
}

void AHTHealthPickup::SpawnInAnimation(const float DeltaTime) const
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

void AHTHealthPickup::FloatingAnimation(const float DeltaTime) const
{
	FVector NewLocation = PickupMesh->GetComponentLocation();
	NewLocation.Z = InitialZ + FMath::Sin(GetGameTimeSinceCreation() * 2) * 50.0f;
	PickupMesh->SetWorldLocation(NewLocation);

	// rotate the mesh constantly
	// FRotator NewRotation = PickupMesh->GetComponentRotation();
	// NewRotation.Yaw += DeltaTime * 100.0f;
	// PickupMesh->SetWorldRotation(NewRotation);	
}

void AHTHealthPickup::DeSpawnAnimation(const float DeltaTime) const
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


