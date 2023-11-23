// Fill out your copyright notice in the Description page of Project Settings.

#include "HoverTank.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HoverTankMovementComponent.h"
#include "MovementReplicatorComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
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
	 * Create Components
	 */
	HoverTankMovementComponent = CreateDefaultSubobject<UHoverTankMovementComponent>(TEXT("Hover Tank Movement Component"));
	MovementReplicatorComponent = CreateDefaultSubobject<UMovementReplicatorComponent>(TEXT("Movement Replicator Component"));
	
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	RootComponent = BoxCollider;

	TankBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tank Base Mesh"));
	TankBaseMesh->SetupAttachment(RootComponent);

	TankCannonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tank Cannon Mesh"));
	TankCannonMesh->SetupAttachment(TankBaseMesh);

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
	SpringArm->SetupAttachment(TankCannonMesh);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	/**
	 * Components Setup
	 */
	SpringArm->SetRelativeRotation(FRotator(-20, 0, 0));
	SpringArm->TargetArmLength = 700;

	Camera->SetRelativeRotation(FRotator(20, 0, 0));

	// Set the BoxColliders collision to BlockAll
	BoxCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	BoxCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	BoxCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	
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
		
		// todo: on shift pressed, strafe
	}
}

// Called every frame
void AHoverTank::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// UE_LOG(LogTemp, Warning, TEXT("Throttle: %f"), Throttle);

	// Draw a debug text above the Pawn showing it's network role
	FString RoleString;
	UEnum::GetValueAsString(GetLocalRole(), RoleString);
	DrawDebugString(GetWorld(), FVector(0, 0, 100), RoleString, this, FColor::White, 0);
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
		
		HoverTankMovementComponent->SetLookUp(-LookAxisVector.Y); // beware! -1 is up, 1 is down
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


