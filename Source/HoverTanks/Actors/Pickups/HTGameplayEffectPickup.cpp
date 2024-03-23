// Fill out your copyright notice in the Description page of Project Settings.


#include "HTGameplayEffectPickup.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"

AHTGameplayEffectPickup::AHTGameplayEffectPickup()
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

	/**
	 * An example of how to reference a UGameplayEffect class from the editor, and set it
	 */
	// static ConstructorHelpers::FClassFinder<UGameplayEffect> HealEffectAsset(TEXT("/Game/HoverTanks/Actors/Pickups/GE_HealEffect_Instant"));
	// HealEffect = HealEffectAsset.Class;
}

void AHTGameplayEffectPickup::BeginPlay()
{
	Super::BeginPlay();

	InitialZ = PickupMesh->GetComponentLocation().Z;

	if (HasAuthority())
	{
		BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &AHTGameplayEffectPickup::OnOverlapBegin);

		bIsSpawningIn = true;

		FVector Height = PickupMesh->GetComponentLocation();
		Height.Z = InitialZ + 400.0f;
		PickupMesh->SetWorldLocation(Height);

		FTimerHandle SpawnInTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(SpawnInTimerHandle, this, &AHTGameplayEffectPickup::SpawnInComplete, 1.0f, false);
	}
}

void AHTGameplayEffectPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHTGameplayEffectPickup, bIsPickedUp);
	DOREPLIFETIME(AHTGameplayEffectPickup, bIsSpawningIn);
}

void AHTGameplayEffectPickup::Tick(float DeltaTime)
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

void AHTGameplayEffectPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		bool bVasValidPickup = false;

		IAbilitySystemInterface* ActorWithAbilitySystem = Cast<IAbilitySystemInterface>(OtherActor);
		if (GameplayEffect != nullptr && ActorWithAbilitySystem != nullptr)
		{
			UAbilitySystemComponent* TargetASC = ActorWithAbilitySystem->GetAbilitySystemComponent();
			
			if (TargetASC != nullptr)
			{
				FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
				EffectContext.AddSourceObject(this);
				EffectContext.AddInstigator(GetInstigatorController(), this);
				
				FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffect, 1.f, EffectContext);
				FGameplayEffectSpec* EffectSpec = EffectSpecHandle.Data.Get();
				
				EffectSpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.EffectMagnitude")), EffectMagnitude);
				if (EffectSpecHandle.IsValid())
				{
					TargetASC->ApplyGameplayEffectSpecToTarget(*EffectSpec, TargetASC);
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

void AHTGameplayEffectPickup::DelayedDestroy()
{
	FTimerHandle DestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AHTGameplayEffectPickup::DoDestroy, 1.0f, false);
}

void AHTGameplayEffectPickup::DoDestroy()
{
	Destroy();
}

void AHTGameplayEffectPickup::SpawnInComplete()
{
	bIsSpawningIn = false;
}

void AHTGameplayEffectPickup::SpawnInAnimation(const float DeltaTime) const
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

void AHTGameplayEffectPickup::FloatingAnimation(const float DeltaTime) const
{
	FVector NewLocation = PickupMesh->GetComponentLocation();
	NewLocation.Z = InitialZ + FMath::Sin(GetGameTimeSinceCreation() * 2) * 50.0f;
	PickupMesh->SetWorldLocation(NewLocation);

	// rotate the mesh constantly
	// FRotator NewRotation = PickupMesh->GetComponentRotation();
	// NewRotation.Yaw += DeltaTime * 100.0f;
	// PickupMesh->SetWorldRotation(NewRotation);	
}

void AHTGameplayEffectPickup::DeSpawnAnimation(const float DeltaTime) const
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


