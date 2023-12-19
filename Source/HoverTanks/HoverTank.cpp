// Fill out your copyright notice in the Description page of Project Settings.

#include "HoverTank.h"

#include "HoverTanks/Components/HealthComponent.h"
#include "HoverTanks/Components/HoverTankMovementComponent.h"
#include "HoverTanks/Components/MovementReplicatorComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/WeaponsComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"

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
	ColliderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Box Collider"));
	RootComponent = ColliderMesh;

	TankBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tank Base Mesh"));
	TankBaseMesh->SetupAttachment(ColliderMesh);

	TankCannonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tank Cannon Mesh"));
	TankCannonMesh->SetupAttachment(ColliderMesh);

	TankBarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tank Barrel Mesh"));
	TankBarrelMesh->SetupAttachment(TankCannonMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ColliderMeshAsset(TEXT("/Game/HoverTanks/HoverTank/HoverTankCollision"));
	UStaticMesh* ColliderMeshAssetObject = ColliderMeshAsset.Object;
	ColliderMesh->SetStaticMesh(ColliderMeshAssetObject);
	
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
	SpringArm->SetupAttachment(ColliderMesh);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	/**
	 * Components Setup
	 */
	SpringArm->TargetArmLength = SpringArmLength;
	SpringArm->AddLocalOffset(FVector(0, 0, SpringArmZOffset));

	// CollisionProfile.Name = "HoverTank" - this is set in the editor
	ColliderMesh->SetCollisionProfileName(CollisionProfile.Name, true);

	// mashes shall not collide
	TankBaseMesh->SetCollisionProfileName(TEXT("NoCollision"));
	TankBaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TankCannonMesh->SetCollisionProfileName(TEXT("NoCollision"));
	TankCannonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TankBarrelMesh->SetCollisionProfileName(TEXT("NoCollision"));
	TankBarrelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/**
	 * HUD
	 */
	static ConstructorHelpers::FClassFinder<UUserWidget> HoverTankHUDWidgetClassFinder(TEXT("/Game/HoverTanks/UI/HUD/WBP_HoverTankHUDWidget"));
	if (HoverTankHUDWidgetClassFinder.Succeeded())
	{
		HoverTankHUDWidgetClass = HoverTankHUDWidgetClassFinder.Class;
	}
}

void AHoverTank::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHoverTank, bIsInputEnabled);
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

void AHoverTank::OnDeath()
{
	// get current player controller
	APlayerController* PlayerController = Cast<APlayerController>(Controller);

	if (PlayerController == nullptr)
	{
		return;
	}
	
	// disable player input
	SetInputEnabled(false);
	ClientRemoveHUDWidget();
}

bool AHoverTank::IsDead() const
{
	if (HealthComponent != nullptr)
	{
		return HealthComponent->IsDead();	
	}

	return false;
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
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (HoverTankHUDWidgetClass)
	{
		HoverTankHUDWidget = CreateWidget<UUserWidget>(GetWorld(), HoverTankHUDWidgetClass);
	}
}

void AHoverTank::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// FString RoleString;
	// UEnum::GetValueAsString(GetLocalRole(), RoleString);
	// UE_LOG(LogTemp, Warning, TEXT("ROLE %s, AHoverTank::PossessedBy"), *RoleString);

	if (GetLocalRole() == ROLE_Authority)
	{
		ClientAddHUDWidget();
	}
}

void AHoverTank::UnPossessed()
{
	Super::UnPossessed();

	// FString RoleString;
	// UEnum::GetValueAsString(GetLocalRole(), RoleString);
	// UE_LOG(LogTemp, Warning, TEXT("ROLE %s, AHoverTank::UnPossessed"), *RoleString);

	if (GetLocalRole() == ROLE_Authority)
	{
		ClientRemoveHUDWidget();
	}
}

void AHoverTank::ClientAddHUDWidget_Implementation()
{
	// UE_LOG(LogTemp, Warning, TEXT("AHoverTank::ClientAddHUDWidget_Implementation"));

	if (HoverTankHUDWidgetClass && HoverTankHUDWidget == nullptr)
	{
		HoverTankHUDWidget = CreateWidget<UUserWidget>(GetWorld(), HoverTankHUDWidgetClass);
	}

	if (HoverTankHUDWidget != nullptr && !HoverTankHUDWidget->IsInViewport())
	{
		HoverTankHUDWidget->AddToViewport();
	}
}

void AHoverTank::ClientRemoveHUDWidget_Implementation()
{
	// UE_LOG(LogTemp, Warning, TEXT("AHoverTank::ClientRemoveHUDWidget_Implementation"));
	
	if (HoverTankHUDWidget != nullptr && HoverTankHUDWidget->IsInViewport())
	{
		HoverTankHUDWidget->RemoveFromParent();
	}
}

void AHoverTank::MoveTriggered(const FInputActionValue& Value)
{
	// UE_LOG(LogTemp, Warning, TEXT("Is input enabled: %s"), bIsInputEnabled ? TEXT("true") : TEXT("false"));
	
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->SetThrottle(MovementVector.Y);
		HoverTankMovementComponent->SetSteering(MovementVector.X);
	}
}

void AHoverTank::MoveCompleted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
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
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->SetIsEBraking(true);
	}
}

void AHoverTank::EBrakeCompleted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->SetIsEBraking(false);
	}
}

void AHoverTank::JumpTriggered()
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("Jump started"));
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->JumpTriggered();
	}
}

void AHoverTank::JumpCompleted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->JumpCompleted();
	}
}

void AHoverTank::BoostTriggered()
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->BoostTriggered();
	}
}

void AHoverTank::BoostCompleted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	if (HoverTankMovementComponent)
	{
		HoverTankMovementComponent->BoostCompleted();
	}
}

void AHoverTank::ShootStarted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
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
