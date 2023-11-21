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

	/**
	 * ROTATE CANNON AND BARREL WITH CAMERA
	 */
	// Rotate the TankCannon mesh based on the LookRight input
	FRotator CannonRotation = TankCannonMesh->GetComponentRotation();
	float CannonYawRotation = LookRight * CannonTurnRate * DeltaTime; // 90 degrees per second
	CannonRotation.Yaw += CannonYawRotation;
	TankCannonMesh->SetWorldRotation(CannonRotation);

	// Rotate the TankBarrel mesh up and down based on LookUp input, with a maximum of 15 degrees up and -10 degrees down
	FRotator BarrelRotation = TankBarrelMesh->GetComponentRotation();
	float BarrelPitchRotation = LookUp * BarrelPitchRate * DeltaTime; // 90 degrees per second
	BarrelRotation.Pitch += BarrelPitchRotation;
	BarrelRotation.Pitch = FMath::Clamp(BarrelRotation.Pitch, -10.0f, 15.0f);
	TankBarrelMesh->SetWorldRotation(BarrelRotation);
}

void AHoverTank::MoveTriggered(const FInputActionValue& Value)
{
	// log out the values in Value
	// UE_LOG(LogTemp, Warning, TEXT("Move Value: %s"), *Value.ToString());

	FVector2D MovementVector = Value.Get<FVector2D>();

	// Throttle = MovementVector.Y;
	// Steering = MovementVector.X;

	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->SetThrottle(MovementVector.Y);
		HoverTankMovementComponent->SetSteering(MovementVector.X);
	}
}

void AHoverTank::MoveCompleted()
{
	// Throttle = 0;
	// Steering = 0;

	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->SetThrottle(0);
		HoverTankMovementComponent->SetSteering(0);
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("Move Completed, Throttle: %f, Steering: %f"), Throttle, Steering);
}

void AHoverTank::LookTriggered(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Look Value: %s"), *Value.ToString());

	FVector2D LookAxisVector = Value.Get<FVector2D>();

	LookUp = -LookAxisVector.Y; // beware!
	LookRight = LookAxisVector.X;
}

void AHoverTank::LookCompleted()
{
	LookUp = 0;
	LookRight = 0;
}


