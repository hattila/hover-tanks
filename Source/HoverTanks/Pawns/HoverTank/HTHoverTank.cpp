// Fill out your copyright notice in the Description page of Project Settings.

#include "HTHoverTank.h"

#include "AbilitySystemComponent.h"
#include "HoverTanks/Components/HTTankMovementComponent.h"
#include "HoverTanks/Components/HTMovementReplicatorComponent.h"
#include "HoverTanks/Components/HTTankEffectsComponent.h"
#include "HoverTanks/Components/HTWeaponsComponent.h"
#include "HoverTanks/Game/HTPSInTeam.h"
#include "HoverTanks/Game/HTPlayerState.h"
#include "HoverTanks/GAS/HTAbilitySystemComponent.h"
#include "HoverTanks/GAS/HTGameplayAbility.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/RectLightComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "HoverTanks/GAS/HTAttributeSetBase.h"
#include "Net/UnrealNetwork.h"

class UNiagaraSystem;

AHTHoverTank::AHTHoverTank()
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
	TankMovementComponent = CreateDefaultSubobject<UHTTankMovementComponent>(TEXT("Tank Movement Component"));
	
	MovementReplicatorComponent = CreateDefaultSubobject<UHTMovementReplicatorComponent>(TEXT("Movement Replicator Component"));
	MovementReplicatorComponent->SetIsReplicated(true);
	
	WeaponsComponent = CreateDefaultSubobject<UHTWeaponsComponent>(TEXT("Weapons Component"));
	WeaponsComponent->SetIsReplicated(true);
	
	TankEffectsComponent = CreateDefaultSubobject<UHTTankEffectsComponent>(TEXT("Tank Effects Component"));
	TankEffectsComponent->SetIsReplicated(true);

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

	/**
	 * Setup defaults, change them in the BP version
	 */
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

	DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
}

void AHTHoverTank::BeginPlay()
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
			Subsystem->AddMappingContext(AbilityInputMappingContext, 0);
		}
	}
}

void AHTHoverTank::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleCameraZoom(DeltaTime);

	if (bShowDebug)
	{
		DebugDrawPlayerTitle();	
	}
}

void AHTHoverTank::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHTHoverTank, bIsInputEnabled);
}

void AHTHoverTank::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHTHoverTank::MoveTriggered);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AHTHoverTank::MoveCompleted);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &AHTHoverTank::MoveCompleted);

		EnhancedInputComponent->BindAction(AltAction, ETriggerEvent::Triggered, this, &AHTHoverTank::AltActionTriggered);
		EnhancedInputComponent->BindAction(AltAction, ETriggerEvent::Completed, this, &AHTHoverTank::AltActionCompleted);

		EnhancedInputComponent->BindAction(StrafeAction, ETriggerEvent::Triggered, this, &AHTHoverTank::StrafeTriggered);
		EnhancedInputComponent->BindAction(StrafeAction, ETriggerEvent::Completed, this, &AHTHoverTank::StrafeCompleted);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHTHoverTank::LookTriggered);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &AHTHoverTank::LookCompleted);

		//EBrake
		EnhancedInputComponent->BindAction(EBrakeAction, ETriggerEvent::Started, this, &AHTHoverTank::EBrakeStarted);
		EnhancedInputComponent->BindAction(EBrakeAction, ETriggerEvent::Completed, this, &AHTHoverTank::EBrakeCompleted);

		//Jump
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AHTHoverTank::JumpTriggered);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AHTHoverTank::JumpCompleted);

		//Boost
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Triggered, this, &AHTHoverTank::BoostTriggered);
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Completed, this, &AHTHoverTank::BoostCompleted);

		//Shoot
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AHTHoverTank::ShootStarted);

		//ZoomIn
		EnhancedInputComponent->BindAction(ZoomInAction, ETriggerEvent::Started, this, &AHTHoverTank::ZoomInActionStarted);
		EnhancedInputComponent->BindAction(ZoomInAction, ETriggerEvent::Completed, this, &AHTHoverTank::ZoomInActionCompleted);

		//Switch weapons
		EnhancedInputComponent->BindAction(NextWeaponAction, ETriggerEvent::Started, this, &AHTHoverTank::NextWeaponActionStarted);
		EnhancedInputComponent->BindAction(PrevWeaponAction, ETriggerEvent::Started, this, &AHTHoverTank::PrevWeaponActionStarted);

		EnhancedInputComponent->BindAction(SelectWeapon01Action, ETriggerEvent::Started, this, &AHTHoverTank::SelectWeapon01ActionStarted);
		EnhancedInputComponent->BindAction(SelectWeapon02Action, ETriggerEvent::Started, this, &AHTHoverTank::SelectWeapon02ActionStarted);

		//Toggle lights
		EnhancedInputComponent->BindAction(ToggleLightsAction, ETriggerEvent::Started, this, &AHTHoverTank::ToggleLightsActionStarted);
		
		
		//Show debug lines and info
		EnhancedInputComponent->BindAction(ShowDebugAction, ETriggerEvent::Started, this, &AHTHoverTank::ShowDebugActionStarted);

		//Suicide
		EnhancedInputComponent->BindAction(SuicideAction, ETriggerEvent::Started, this, &AHTHoverTank::SuicideActionStarted);

		BindAbilitySystemComponentActions();
	}
	
}

void AHTHoverTank::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitPlayer();
	BindAbilitySystemComponentActions();
}

UAbilitySystemComponent* AHTHoverTank::GetAbilitySystemComponent() const
{
	// get the ability system component from the player state
	AHTPlayerState* HTPlayerState = GetPlayerState<AHTPlayerState>();
	if (HTPlayerState)
	{
		return HTPlayerState->GetAbilitySystemComponent();
	}

	UE_LOG(LogTemp, Warning, TEXT("AHTHoverTank::GetAbilitySystemComponent: HTPlayerState is null!"));
	return nullptr;
}

void AHTHoverTank::OnDeath()
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
	
	if (DeathEffect != nullptr)
	{
		// add DeathEffect, one liner edition
		AbilitySystemComponent->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(DeathEffect->GetDefaultObject()), 1.0f, AbilitySystemComponent->MakeEffectContext());	
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("AHTHoverTank::OnDeath: DeathEffect is null set it in the Blueprint!"));
	}
	
	// disable player input
	SetInputEnabled(false);

	OnTankDeath.Broadcast();
	ClientBroadcastOnTankDeath(); // eg: notify the HUD
	
	ColliderMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	// ECC_GameTraceChannel1 is the Projectile channel
	ColliderMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);

	if (TankEffectsComponent)
	{
		TankEffectsComponent->OnDeath();
	}
	
	// change the mesh to a wreckage
}

bool AHTHoverTank::IsDead() const
{
	AHTPlayerState* HTPlayerState = GetPlayerState<AHTPlayerState>();
	if (HTPlayerState)
	{
		return HTPlayerState->IsDead();
	}

	return false;
}

void AHTHoverTank::ServerSuicide_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	if (DamageEffect == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AHTHoverTank::Suicide - No DamageEffect specified")); // tank should be killed anyway
		return;
	}

	FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponent()->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	EffectContext.AddInstigator(GetController(), GetController());

	FGameplayEffectSpecHandle DamageEffectSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(DamageEffect, 1.f, EffectContext);
	FGameplayEffectSpec* DamageEffectSpec = DamageEffectSpecHandle.Data.Get();

	DamageEffectSpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), 9999);
	if (DamageEffectSpecHandle.IsValid())
	{
		GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*DamageEffectSpec, GetAbilitySystemComponent());
	}
}

void AHTHoverTank::ClientBroadcastOnTankDeath_Implementation()
{
	OnTankDeath.Broadcast();
}

FHitResult AHTHoverTank::FindTargetAtCrosshair() const
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
void AHTHoverTank::ApplyTeamColors(UHTTeamDataAsset* TeamDataAsset)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyTeamColors can only be called on the server!"));
		return;
	}
	
	if (TankEffectsComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("TankEffectsComponent is null on HoverTank, cannot apply team colors"));
		return;
	}

	TankEffectsComponent->ApplyTeamColors(TeamDataAsset);
}

/**
 * ILaunchable interface 
 */
void AHTHoverTank::DirectionalLaunch(const FVector& LaunchVelocity)
{
	if (TankMovementComponent)
	{
		TankMovementComponent->SetDirectionalLaunchVelocity(LaunchVelocity);
	}
}

/**
 * Is called on Possessed on the server and Onrep_PlayerState on the client
 */
void AHTHoverTank::InitPlayer()
{
	// Player State should have the AbilitySystemComponent
	AHTPlayerState* HTPlayerState = GetPlayerState<AHTPlayerState>();
	if (!HTPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitPlayer: HTPlayerState is null"));
		return;
	}
	
	AbilitySystemComponent = Cast<UHTAbilitySystemComponent>(HTPlayerState->GetAbilitySystemComponent());
	
	// UE_LOG(LogTemp, Warning, TEXT("InitPlayer: %s"), *HTPlayerState->GetPlayerName());
	AbilitySystemComponent->InitAbilityActorInfo(HTPlayerState, this);

	// if there is an ongoing DeathEffect, remove it
	if (AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(DeadTag));
	}

	InitializeAttributes();
	AddOngoingEffects();
	AddDefaultAbilities();
}

void AHTHoverTank::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitPlayer();
}

void AHTHoverTank::UnPossessed()
{
	if (WeaponsComponent)
	{
		WeaponsComponent->MulticastDestroyAttachedWeapons();
	}
	
	Super::UnPossessed();
}

void AHTHoverTank::InitializeAttributes()
{
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	if (!DefaultAttributes)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing DefaultAttributes for %s. Please fill in the character's Blueprint."), *FString(__FUNCTION__), *GetName());
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	// apply the default attributes, Tank is lvl 1, no lvl up in this game yet
	FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributes, 1, EffectContext);
	if (EffectSpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void AHTHoverTank::AddOngoingEffects()
{
	if (GetLocalRole() != ROLE_Authority || !IsValid(AbilitySystemComponent) || AbilitySystemComponent->bOngoingEffectsApplied)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (TSubclassOf<UGameplayEffect> GameplayEffect : OngoingEffects)
	{
		FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
		if (EffectSpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		}
	}

	AbilitySystemComponent->bOngoingEffectsApplied = true;
}

void AHTHoverTank::AddDefaultAbilities()
{
	if (HasAuthority() && !AbilitySystemComponent->bCharacterAbilitiesGiven)
	{
		// Give the player every ability in the DefaultAbilities array
		for (TSubclassOf<UGameplayAbility> Ability : DefaultAbilities)
		{
			FGameplayAbilitySpecHandle AbilitySpecHandle = AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(Ability, 0, -1)
			);
		}

		AbilitySystemComponent->bCharacterAbilitiesGiven = true;
	}
}

void AHTHoverTank::BindAbilitySystemComponentActions()
{
	if (bIsAbilitySystemComponentInputBound)
	{
		// UE_LOG(LogTemp, Warning, TEXT("AbilitySystemComponent actions are already bound"));
		return;
	}
	
	if (!IsValid(AbilitySystemComponent))
	{
		// UE_LOG(LogTemp, Warning, TEXT("BindAbilitySystemComponentActions: AbilitySystemComponent is null"));
		return;
	}

	if (!IsValid(InputComponent))
	{
		// UE_LOG(LogTemp, Warning, TEXT("BindAbilitySystemComponentActions: InputComponent is null"));
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(AbilityOneInputAction, ETriggerEvent::Started, this, &AHTHoverTank::AbilityOneStartedAction);
		EnhancedInputComponent->BindAction(AbilityTwoInputAction, ETriggerEvent::Started, this, &AHTHoverTank::AbilityTwoStartedAction);
	
		bIsAbilitySystemComponentInputBound = true;	
	}
}

void AHTHoverTank::MoveTriggered(const FInputActionValue& Value)
{
	// UE_LOG(LogTemp, Warning, TEXT("Is input enabled: %s"), bIsInputEnabled ? TEXT("true") : TEXT("false"));
	// UE_LOG(LogTemp, Warning, TEXT("MoveTriggered Value: %s"), *Value.ToString());
	
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (TankMovementComponent)
	{
		TankMovementComponent->SetThrottle(MovementVector.Y);
		TankMovementComponent->SetSteering(MovementVector.X);
	}
}

void AHTHoverTank::MoveCompleted()
{
	// UE_LOG(LogTemp, Warning, TEXT("Move Completed"));
	
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	if (TankMovementComponent)
	{
		TankMovementComponent->SetThrottle(0);
		TankMovementComponent->SetSteering(0);
	}
}

/**
 * A move does not get Completed if the modifier key is pressed or released, but it does get Canceled
 */
void AHTHoverTank::MoveCanceled()
{
	// UE_LOG(LogTemp, Warning, TEXT("Move Canceled"));
	MoveCompleted(); // Maybe I should only nullify the steering, but not the throttle?
}

void AHTHoverTank::AltActionTriggered(const FInputActionValue& Value)
{
	// UE_LOG(LogTemp, Warning, TEXT("AltAction Value: %s"), *Value.ToString());
}

void AHTHoverTank::AltActionCompleted()
{
	// UE_LOG(LogTemp, Warning, TEXT("AltAction Completed"));
	StrafeCompleted();
}

void AHTHoverTank::StrafeTriggered(const FInputActionValue& Value)
{
	// UE_LOG(LogTemp, Warning, TEXT("Strafe Value: %s"), *Value.ToString());

	if (bIsInputEnabled == false)
	{
		return;
	}

	const FVector2D StrafeVector = Value.Get<FVector2D>();

	if (TankMovementComponent)
	{
		TankMovementComponent->SetSteering(0);
		TankMovementComponent->SetSideStrafeThrottle(StrafeVector.X);
	}
}

void AHTHoverTank::StrafeCompleted()
{
	// UE_LOG(LogTemp, Warning, TEXT("Strafe Completed"));

	if (bIsInputEnabled == false)
	{
		return;
	}

	if (TankMovementComponent)
	{
		TankMovementComponent->SetSideStrafeThrottle(0);
	}
}


void AHTHoverTank::LookTriggered(const FInputActionValue& Value)
{
	// UE_LOG(LogTemp, Warning, TEXT("Look Value: %s"), *Value.ToString());
	
	if (TankMovementComponent)
	{
		FVector2D LookAxisVector = Value.Get<FVector2D>();
		
		TankMovementComponent->SetLookUp(LookAxisVector.Y);
		TankMovementComponent->SetLookRight(LookAxisVector.X);
	}
}

void AHTHoverTank::LookCompleted()
{
	if (TankMovementComponent)
	{
		TankMovementComponent->SetLookUp(0);
		TankMovementComponent->SetLookRight(0);
	}
}

void AHTHoverTank::EBrakeStarted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	if (TankMovementComponent)
	{
		TankMovementComponent->SetIsEBraking(true);
	}
}

void AHTHoverTank::EBrakeCompleted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	if (TankMovementComponent)
	{
		TankMovementComponent->SetIsEBraking(false);
	}
}

void AHTHoverTank::JumpTriggered()
{
	// UE_LOG(LogTemp, Warning, TEXT("Is input enabled: %s"), bIsInputEnabled ? TEXT("true") : TEXT("false"));

	if (bIsInputEnabled == false)
	{
		return;
	}

	if (TankMovementComponent)
	{
		TankMovementComponent->JumpTriggered();
	}
}

void AHTHoverTank::JumpCompleted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}

	if (TankMovementComponent)
	{
		TankMovementComponent->JumpCompleted();
	}
}

void AHTHoverTank::BoostTriggered()
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	if (TankMovementComponent)
	{
		TankMovementComponent->BoostTriggered();
	}
}

void AHTHoverTank::BoostCompleted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	if (TankMovementComponent)
	{
		TankMovementComponent->BoostCompleted();
	}
}

void AHTHoverTank::ShootStarted()
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

void AHTHoverTank::ZoomInActionStarted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}

	bIsZoomedIn = true;
}

void AHTHoverTank::ZoomInActionCompleted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}

	bIsZoomedIn = false;
}

void AHTHoverTank::ShowDebugActionStarted()
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

void AHTHoverTank::SuicideActionStarted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}
	
	ServerSuicide();
}

void AHTHoverTank::HandleCameraZoom(const float DeltaTime) const
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

void AHTHoverTank::NextWeaponActionStarted(const FInputActionValue& Value)
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

void AHTHoverTank::PrevWeaponActionStarted(const FInputActionValue& Value)
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

void AHTHoverTank::SelectWeapon01ActionStarted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}

	if (WeaponsComponent)
	{
		WeaponsComponent->SwitchToCannon();
	}
}

void AHTHoverTank::SelectWeapon02ActionStarted()
{
	if (bIsInputEnabled == false)
	{
		return;
	}

	if (WeaponsComponent)
	{
		WeaponsComponent->SwitchToRockets();
	}
}

void AHTHoverTank::ToggleLightsActionStarted()
{
	// UE_LOG(LogTemp, Warning, TEXT("ToggleLightsActionStarted"));
	
	if (bIsInputEnabled == false)
	{
		return;
	}

	if (TankEffectsComponent)
	{
		TankEffectsComponent->ServerToggleLights();
	}
}

void AHTHoverTank::DebugDrawPlayerTitle()
{
	FString RoleString;
	UEnum::GetValueAsString(GetLocalRole(), RoleString);

	AHTPlayerState* HTPlayerState = GetPlayerState<AHTPlayerState>();
	FString PlayerName = HTPlayerState ? HTPlayerState->GetPlayerName() : "No Player State";

	if (HTPlayerState)
	{
		AHTPSInTeam* InTeamPlayerState = Cast<AHTPSInTeam>(HTPlayerState);
		if (InTeamPlayerState)
		{
			PlayerName += FString::Printf(TEXT(" (Team %d)"), InTeamPlayerState->GetTeamId());
		}
	}
	
	FString DebugString = FString::Printf(TEXT("%s\nRole: %s, SH: %.0f / HP: %.0f"), *PlayerName, *RoleString, HTPlayerState->GetAttributeSetBase()->GetShield(), HTPlayerState->GetAttributeSetBase()->GetHealth());
	DrawDebugString(GetWorld(), FVector(0, 0, 150), DebugString, this, FColor::White, 0);
}

void AHTHoverTank::AbilityOneStartedAction()
{
	// UE_LOG(LogTemp, Warning, TEXT("AbilityOneStartedAction"));

	TArray<FGameplayAbilitySpec> AbilitySpecs = AbilitySystemComponent->GetActivatableAbilities();
	if (AbilitySpecs.Num() > 0)
	{
		// try to activate the first ability
		AbilitySystemComponent->TryActivateAbility(AbilitySpecs[0].Handle, true);
	}
}

void AHTHoverTank::AbilityTwoStartedAction()
{
	// UE_LOG(LogTemp, Warning, TEXT("AbilityTwoStartedAction"));

	TArray<FGameplayAbilitySpec> AbilitySpecs = AbilitySystemComponent->GetActivatableAbilities();
	if (AbilitySpecs.Num() > 0 && AbilitySpecs.IsValidIndex(1))
	{
		AbilitySystemComponent->TryActivateAbility(AbilitySpecs[1].Handle, true);
	}
	
}