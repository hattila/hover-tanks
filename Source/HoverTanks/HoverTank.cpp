// Fill out your copyright notice in the Description page of Project Settings.

#include "HoverTank.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HealthComponent.h"
#include "HoverTankMovementComponent.h"
#include "MovementReplicatorComponent.h"
#include "TankProjectile.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WeaponsComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AHoverTank::AHoverTank()
{
	/**
	  * Actor setup
	  */
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	bUseControllerRotationYaw = false;

	/**
	 * Create ActorComponents
	 */
	HoverTankMovementComponent = CreateDefaultSubobject<UHoverTankMovementComponent>(TEXT("Hover Tank Movement Component"));

	MovementReplicatorComponent = CreateDefaultSubobject<UMovementReplicatorComponent>(TEXT("Movement Replicator Component"));
	MovementReplicatorComponent->SetIsReplicated(true);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
	HealthComponent->SetIsReplicated(true);

	WeaponsComponent = CreateDefaultSubobject<UWeaponsComponent>(TEXT("Weapons Component"));

	/**
	 * Create Visible Components
	 */
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	RootComponent = BoxCollider;

	TankBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tank Base Mesh"));
	TankBaseMesh->SetupAttachment(BoxCollider);

	TankCannonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tank Cannon Mesh"));
	TankCannonMesh->SetupAttachment(BoxCollider);

	TankBarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tank Barrel Mesh"));
	TankBarrelMesh->SetupAttachment(TankCannonMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> TankBaseMeshAsset(TEXT("/Game/HoverTanks/HoverTank/HoverTank_TankBase"));
	UStaticMesh* TankBaseMeshAssetObject = TankBaseMeshAsset.Object;
	TankBaseMesh->SetStaticMesh(TankBaseMeshAssetObject);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> TankCannonMeshAsset(TEXT("/Game/HoverTanks/HoverTank/HoverTank_TankCannon"));
	UStaticMesh* TankCannonMeshAssetObject = TankCannonMeshAsset.Object;
	TankCannonMesh->SetStaticMesh(TankCannonMeshAssetObject);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> TankBarrelMeshAsset(TEXT("/Game/HoverTanks/HoverTank/HoverTank_TankCannonBarrel"));
	UStaticMesh* TankBarrelMeshAssetObject = TankBarrelMeshAsset.Object;
	TankBarrelMesh->SetStaticMesh(TankBarrelMeshAssetObject);
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(BoxCollider);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	/**
	 * Components Setup
	 */
	SpringArm->TargetArmLength = SpringArmLength;
	SpringArm->AddLocalOffset(FVector(0, 0, SpringArmZOffset));

	// Set the BoxColliders collision to BlockAll
	BoxCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxCollider->SetCollisionResponseToAllChannels(ECR_Block);
	BoxCollider->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	BoxCollider->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	
}

// Called when the game starts or when spawned
void AHoverTank::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(false);
	
	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AHoverTank::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHoverTank::MoveTriggered);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AHoverTank::MoveCompleted);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHoverTank::LookTriggered);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &AHoverTank::LookCompleted);

		//EBrake
		EnhancedInputComponent->BindAction(EBrakeAction, ETriggerEvent::Started, this, &AHoverTank::EBrakeStarted);
		EnhancedInputComponent->BindAction(EBrakeAction, ETriggerEvent::Completed, this, &AHoverTank::EBrakeCompleted);

		//Jump
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AHoverTank::JumpTriggered);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AHoverTank::JumpCompleted);

		//Boost
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Triggered, this, &AHoverTank::BoostTriggered);
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Completed, this, &AHoverTank::BoostCompleted);

		//Shoot
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AHoverTank::ShootStarted);
		
		// todo: on shift pressed, strafe
	}
}

// Called every frame
void AHoverTank::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// UE_LOG(LogTemp, Warning, TEXT("Throttle: %f"), Throttle);

	DebugDrawPlayerTitle();
	if (IsLocallyControlled())
	{
		DebugDrawSphereAsCrosshair();	
	}
}

void AHoverTank::MoveTriggered(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->SetThrottle(MovementVector.Y);
		HoverTankMovementComponent->SetSteering(MovementVector.X);
	}
}

void AHoverTank::MoveCompleted()
{
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->SetThrottle(0);
		HoverTankMovementComponent->SetSteering(0);
	}
}

void AHoverTank::LookTriggered(const FInputActionValue& Value)
{
	// UE_LOG(LogTemp, Warning, TEXT("Look Value: %s"), *Value.ToString());

	if (HoverTankMovementComponent)
	{
		FVector2D LookAxisVector = Value.Get<FVector2D>();
		
		HoverTankMovementComponent->SetLookUp(LookAxisVector.Y); // beware! -1 is up, 1 is down
		HoverTankMovementComponent->SetLookRight(LookAxisVector.X);
	}
}

void AHoverTank::LookCompleted()
{
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->SetLookUp(0);
		HoverTankMovementComponent->SetLookRight(0);
	}
}

void AHoverTank::EBrakeStarted()
{
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->SetIsEBraking(true);
	}
}

void AHoverTank::EBrakeCompleted()
{
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->SetIsEBraking(false);
	}
}

void AHoverTank::JumpTriggered()
{
	// UE_LOG(LogTemp, Warning, TEXT("Jump started"));
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->JumpTriggered();
	}
}

void AHoverTank::JumpCompleted()
{
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->JumpCompleted();
	}
}

void AHoverTank::BoostTriggered()
{
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->BoostTriggered();
	}
}

void AHoverTank::BoostCompleted()
{
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->BoostCompleted();
	}
}

void AHoverTank::ShootStarted()
{
	if (WeaponsComponent)
	{
		WeaponsComponent->AttemptToShoot();
	}
}

void AHoverTank::DebugDrawPlayerTitle()
{
	FString RoleString;
	UEnum::GetValueAsString(GetLocalRole(), RoleString);

	APlayerState* CurrentPlayerState = GetPlayerState();
	FString PlayerName = CurrentPlayerState ? CurrentPlayerState->GetPlayerName() : "No Player State";
	FString DebugString = FString::Printf(TEXT("%s\nRole: %s, HP: %.0f"), *PlayerName, *RoleString,  HealthComponent->GetHealth());
	DrawDebugString(GetWorld(), FVector(0, 0, 100), DebugString, this, FColor::White, 0);
}

void AHoverTank::DebugDrawSphereAsCrosshair() const
{
	FHitResult Hit;
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * 20000;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params);

	// Scale up the sphere radius based on the distance from the camera. The greater the distance, the larger the radius
	float SphereRadius = FMath::Clamp((Hit.Location - Start).Size() / 100, 25.f, 100.f);
	DrawDebugSphere(GetWorld(), Hit.Location, SphereRadius, 12, FColor::Yellow, false, 0.f, 0, 3.f);
}
