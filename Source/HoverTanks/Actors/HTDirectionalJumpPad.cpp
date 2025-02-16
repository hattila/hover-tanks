// Fill out your copyright notice in the Description page of Project Settings.


#include "HTDirectionalJumpPad.h"

#include "Components/BoxComponent.h"
#include "HoverTanks/Pawns/HoverTank/HTHoverTank.h"


// Sets default values
AHTDirectionalJumpPad::AHTDirectionalJumpPad()
{
	/**
	 * No tick is needed, as the jump pad is a static object, will add an impulse on collision
	 */
	PrimaryActorTick.bCanEverTick = false;

	// initialize the box collider
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	// BoxCollider->SetCollisionProfileName(TEXT("PickupSpawner"));
	BoxCollider->SetBoxExtent(FVector(200.0f, 200.0f, 200.0f));
	RootComponent = BoxCollider;

	// initialize the baseMesh
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);

	BaseMesh->SetCollisionProfileName(TEXT("NoCollision"));
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BaseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0));
}

// Called when the game starts or when spawned
void AHTDirectionalJumpPad::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &AHTDirectionalJumpPad::OnOverlapBegin);
		BoxCollider->OnComponentEndOverlap.AddDynamic(this, &AHTDirectionalJumpPad::OnOverlapEnd);
	}
}

void AHTDirectionalJumpPad::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FVector LaunchDirection = GetActorForwardVector();
	LaunchDirection.Z = JumpVectorUpwardComponent;
	LaunchDirection.Normalize();

	// Cast to the HoverTank, and launch it in the direction of the LaunchDirection while this is PoC
	// Later, we might have a Launchable interface
	AHTHoverTank* HoverTank = Cast<AHTHoverTank>(OtherActor);
	if (HoverTank)
	{
		HoverTank->DirectionalLaunch(LaunchDirection * JumpForce);
	}
	
}

void AHTDirectionalJumpPad::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// log out that overlap ended with OtherActor
	UE_LOG(LogTemp, Warning, TEXT("Overlap ended with %s"), *OtherActor->GetName());
}
