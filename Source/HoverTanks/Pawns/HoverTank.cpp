// Fill out your copyright notice in the Description page of Project Settings.

#include "HoverTank.h"

#include "HoverTanks/Components/HealthComponent.h"
#include "HoverTanks/Components/HoverTankMovementComponent.h"
#include "HoverTanks/Components/MovementReplicatorComponent.h"
#include "HoverTanks/Components/HoverTankEffectsComponent.h"
#include "HoverTanks/Components/WeaponsComponent.h"
#include "HoverTanks/Game/InTeamPlayerState.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/RectLightComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

class UNiagaraSystem;

AHoverTank::AHoverTank()
{
	/**
	 * Actor setup
	 */
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	bAlwaysRelevant = true;

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
	WeaponsComponent->SetIsReplicated(true);

	HoverTankEffectsComponent = CreateDefaultSubobject<UHoverTankEffectsComponent>(TEXT("Hover Tank Effects Component"));
	HoverTankEffectsComponent->SetIsReplicated(true);

	/**
	 * Create Visible Components
	 */
	ColliderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Collider"));
	RootComponent = ColliderMesh;

	TankBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tank Base Mesh"));
	TankBaseMesh->SetupAttachment(ColliderMesh);

	TankCannonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tank Cannon Mesh"));
	TankCannonMesh->SetupAttachment(ColliderMesh);

	TankBarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tank Barrel Mesh"));
	TankBarrelMesh->SetupAttachment(TankCannonMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ColliderMeshAsset(TEXT("/Game/HoverTanks/Pawns/HoverTankCollision"));
	UStaticMesh* ColliderMeshAssetObject = ColliderMeshAsset.Object;
	ColliderMesh->SetStaticMesh(ColliderMeshAssetObject);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TankBaseMeshAsset(TEXT("/Game/HoverTanks/Pawns/HoverTank_TankBase"));
	UStaticMesh* TankBaseMeshAssetObject = TankBaseMeshAsset.Object;
	TankBaseMesh->SetStaticMesh(TankBaseMeshAssetObject);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> TankCannonMeshAsset(TEXT("/Game/HoverTanks/Pawns/HoverTank_TankCannon"));
	UStaticMesh* TankCannonMeshAssetObject = TankCannonMeshAsset.Object;
	TankCannonMesh->SetStaticMesh(TankCannonMeshAssetObject);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> TankBarrelMeshAsset(TEXT("/Game/HoverTanks/Pawns/HoverTank_TankCannonBarrel"));
	UStaticMesh* TankBarrelMeshAssetObject = TankBarrelMeshAsset.Object;
	TankBarrelMesh->SetStaticMesh(TankBarrelMeshAssetObject);
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(ColliderMesh);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	TankLights = CreateDefaultSubobject<URectLightComponent>(TEXT("Tank Lights"));
	TankLights->SetupAttachment(ColliderMesh);

	/**
	 * Components Setup
	 */
	SpringArm->TargetArmLength = SpringArmLength;
	SpringArm->AddLocalOffset(FVector(0, 0, SpringArmZOffset));
	SpringArm->bUsePawnControlRotation = true;

	TankLights->SetIntensity(50000.f);
	TankLights->SetAttenuationRadius(2000.f);
	TankLights->SetSourceHeight(16);
	TankLights->SetSourceWidth(32);
	TankLights->SetBarnDoorAngle(30);
	TankLights->SetBarnDoorLength(30);
	TankLights->SetRelativeLocation(FVector(216.317645f, 0.f, 4.462727f));
	
	ColliderMesh->SetCollisionProfileName(TEXT("HoverTank"), true);
	ColliderMesh->SetVisibility(false);

	// mashes shall not collide
	TankBaseMesh->SetCollisionProfileName(TEXT("NoCollision"));
	TankBaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TankCannonMesh->SetCollisionProfileName(TEXT("NoCollision"));
	TankCannonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TankBarrelMesh->SetCollisionProfileName(TEXT("NoCollision"));
	TankBarrelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/**
	 * Materials
	 */
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TankBaseMaterialAsset(TEXT("/Game/Megascans/surfaces/Painted_Gun_Metal_shrbehqc/MI_Painted_Gun_Metal_shrbehqc_4K"));
	UMaterialInterface* TankBaseMaterialAssetObject = TankBaseMaterialAsset.Object;
	TankBaseMesh->SetMaterial(0, TankBaseMaterialAssetObject);
	TankCannonMesh->SetMaterial(0, TankBaseMaterialAssetObject);
	TankBarrelMesh->SetMaterial(0, TankBaseMaterialAssetObject);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TankLightsMaterialAsset(TEXT("/Game/HoverTanks/Materials/MI_HoverTankLights"));
	UMaterialInterface* TankLightsMaterialAssetObject = TankLightsMaterialAsset.Object;
	TankBaseMesh->SetMaterial(1, TankLightsMaterialAssetObject);
	TankCannonMesh->SetMaterial(1, TankLightsMaterialAssetObject);
	TankBarrelMesh->SetMaterial(1, TankLightsMaterialAssetObject);
}

void AHoverTank::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(false);
	
	if (HasAuthority())
	{
		// NetUpdateFrequency = 1;
	}
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AHoverTank::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleCameraZoom(DeltaTime);

	if (bShowDebug)
	{
		DebugDrawPlayerTitle();	
	}
}

void AHoverTank::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHoverTank, bIsInputEnabled);
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

		//ZoomIn
		EnhancedInputComponent->BindAction(ZoomInAction, ETriggerEvent::Started, this, &AHoverTank::ZoomInActionStarted);
		EnhancedInputComponent->BindAction(ZoomInAction, ETriggerEvent::Completed, this, &AHoverTank::ZoomInActionCompleted);

		//Switch weapons
		EnhancedInputComponent->BindAction(NextWeaponAction, ETriggerEvent::Started, this, &AHoverTank::NextWeaponActionStarted);
		EnhancedInputComponent->BindAction(PrevWeaponAction, ETriggerEvent::Started, this, &AHoverTank::PrevWeaponActionStarted);

		//Toggle lights
		EnhancedInputComponent->BindAction(ToggleLightsAction, ETriggerEvent::Started, this, &AHoverTank::ToggleLightsActionStarted);
		
		
		//Show debug lines and info
		EnhancedInputComponent->BindAction(ShowDebugAction, ETriggerEvent::Started, this, &AHoverTank::ShowDebugActionStarted);
		
	}
}

void AHoverTank::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void AHoverTank::OnDeath()
{
	if (!HasAuthority())
	{
		return;
	}
	
	// do not do anything if this is an uncontrolled tank
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController == nullptr)
	{
		return;
	}
	
	// disable player input
	SetInputEnabled(false);

	OnTankDeath.Broadcast();
	ClientBroadcastOnTankDeath(); // eg: notify the HUD
	
	ColliderMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	if (HoverTankEffectsComponent)
	{
		HoverTankEffectsComponent->OnDeath();
	}
	
	// change the mesh to a wreckage
}

bool AHoverTank::IsDead() const
{
	if (HealthComponent != nullptr)
	{
		return HealthComponent->IsDead();	
	}

	return false;
}

void AHoverTank::ClientBroadcastOnTankDeath_Implementation()
{
	OnTankDeath.Broadcast();
}

FHitResult AHoverTank::FindTargetAtCrosshair() const
{
	FHitResult Hit;
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * 40000;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// line trace with the custom FindTarget channel
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_GameTraceChannel2, Params);
	
	return Hit;
}

/**
 * IHasTeamColors interface 
 */
void AHoverTank::ApplyTeamColors(UTeamDataAsset* TeamDataAsset)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyTeamColors can only be called on the server!"));
		return;
	}
	
	if (HoverTankEffectsComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("HoverTankEffectsComponent is null on HoverTank, cannot apply team colors"));
		return;
	}

	HoverTankEffectsComponent->ApplyTeamColors(TeamDataAsset);
}

void AHoverTank::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AHoverTank::UnPossessed()
{
	if (WeaponsComponent)
	{
		WeaponsComponent->MulticastDestroyAttachedWeapons();
	}
	
	Super::UnPossessed();
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
		
		HoverTankMovementComponent->SetLookUp(LookAxisVector.Y);
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
	// UE_LOG(LogTemp, Warning, TEXT("Is input enabled: %s"), bIsInputEnabled ? TEXT("true") : TEXT("false"));
	
	if (bIsInputEnabled == false)
	{
		return;
	}

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
		const FHitResult Hit = FindTargetAtCrosshair();
		WeaponsComponent->AttemptToShoot(Hit);
	}
}

void AHoverTank::ZoomInActionStarted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}

	bIsZoomedIn = true;
}

void AHoverTank::ZoomInActionCompleted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}

	bIsZoomedIn = false;
}

void AHoverTank::ShowDebugActionStarted()
{
	bShowDebug = !bShowDebug;

	if (bShowDebug)
	{
		ColliderMesh->SetVisibility(true);
	}
	else
	{
		ColliderMesh->SetVisibility(false);
	}
}

void AHoverTank::HandleCameraZoom(const float DeltaTime) const
{
	if (bIsZoomedIn)
	{
		Camera->SetFieldOfView(FMath::FInterpTo(Camera->FieldOfView, ZoomedInCameraZoomFOV, DeltaTime, 5.f));
	}
	else
	{
		Camera->SetFieldOfView(FMath::FInterpTo(Camera->FieldOfView, DefaultCameraZoomFOV, DeltaTime, 5.f));
	}
}

void AHoverTank::NextWeaponActionStarted(const FInputActionValue& Value)
{
	if (bIsInputEnabled == false)
	{
		return;
	}

	// UE_LOG(LogTemp, Warning, TEXT("NextWeaponActionStarted %s"), *Value.ToString());
	
	if (WeaponsComponent)
	{
		WeaponsComponent->SwitchToNextWeapon();
	}
}

void AHoverTank::PrevWeaponActionStarted(const FInputActionValue& Value)
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("NextWeaponActionStarted %s"), *Value.ToString());

	if (WeaponsComponent)
	{
		WeaponsComponent->SwitchToPrevWeapon();
	}
}

void AHoverTank::ToggleLightsActionStarted()
{
	// UE_LOG(LogTemp, Warning, TEXT("ToggleLightsActionStarted"));
	
	if (bIsInputEnabled == false)
	{
		return;
	}

	if (HoverTankEffectsComponent)
	{
		HoverTankEffectsComponent->ServerToggleLights();
	}
}

void AHoverTank::DebugDrawPlayerTitle()
{
	FString RoleString;
	UEnum::GetValueAsString(GetLocalRole(), RoleString);

	APlayerState* CurrentPlayerState = GetPlayerState();
	FString PlayerName = CurrentPlayerState ? CurrentPlayerState->GetPlayerName() : "No Player State";

	if (CurrentPlayerState)
	{
		AInTeamPlayerState* InTeamPlayerState = Cast<AInTeamPlayerState>(CurrentPlayerState);
		if (InTeamPlayerState)
		{
			PlayerName += FString::Printf(TEXT(" (Team %d)"), InTeamPlayerState->GetTeamId());
		}
	}
	
	FString DebugString = FString::Printf(TEXT("%s\nRole: %s, HP: %.0f"), *PlayerName, *RoleString,  HealthComponent->GetHealth());
	DrawDebugString(GetWorld(), FVector(0, 0, 150), DebugString, this, FColor::White, 0);
}
