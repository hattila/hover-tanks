// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "HoverTanks/Actors/Launchable.h"
#include "HoverTanks/Pawns/HasTeamColors.h"
#include "HTHoverTank.generated.h"

class UHTGameplayAbility;
class UHTAbilitySystemComponent;
class UHTWeaponsComponent;
class UHTTankEffectsComponent;
class UHTTankMovementComponent;
class UHTMovementReplicatorComponent;

class UGameplayEffect;
class UGameplayAbility;
class URectLightComponent;
class UNiagaraComponent;
class USphereComponent;
struct FInputActionValue;
class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTankDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwitched, int32, NewWeapon);

UCLASS()
class HOVERTANKS_API AHTHoverTank :
	public APawn,
	public IAbilitySystemInterface,
	public IHasTeamColors,
	public ILaunchable
{
	GENERATED_BODY()
	
public:
	AHTHoverTank();
	
	UPROPERTY(BlueprintAssignable, Category = "CustomEvents")
	FOnTankDeath OnTankDeath;

	UPROPERTY(BlueprintAssignable, Category = "CustomEvents")
	FOnWeaponSwitched OnWeaponSwitched;

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	virtual void OnRep_PlayerState() override;

	//~ IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ IAbilitySystemInterface

	UFUNCTION(BlueprintPure)
	UStaticMeshComponent* GetTankBaseMesh() const { return TankBaseMesh; }
	
	UFUNCTION(BlueprintPure)
	UStaticMeshComponent* GetTankCannonMesh() const { return TankCannonMesh; }

	UFUNCTION(BlueprintPure)
	UStaticMeshComponent* GetTankBarrelMesh() const { return TankBarrelMesh; }

	UFUNCTION(BlueprintPure)
	URectLightComponent* GetTankLights() const { return TankLights; }

	void OnDeath();
	bool IsDead() const;

	UFUNCTION(Server, Reliable)
	void ServerSuicide();
	
	bool IsInputEnabled() const { return bIsInputEnabled; }
	void SetInputEnabled(const bool bNewInputEnabled) { bIsInputEnabled = bNewInputEnabled; }
	bool GetShowDebug() const { return bShowDebug; }

	UFUNCTION(Client, Unreliable)
	void ClientBroadcastOnTankDeath();

	FHitResult FindTargetAtCrosshair() const;
	
	UHTWeaponsComponent* GetWeaponsComponent() const { return WeaponsComponent; }
	UHTTankEffectsComponent* GetEffectsComponent() const { return TankEffectsComponent; }

	//~ Begin IHasTeamColors interface
	virtual void ApplyTeamColors(UHTTeamDataAsset* TeamDataAsset) override;
	//~ End of IHasTeamColors interface

	//~ Begin ILaunchable interface
	virtual void DirectionalLaunch(const FVector& LaunchVelocity) override;
	//~ End ILaunchable interface
	
protected:
	virtual void BeginPlay() override;

	void InitPlayer();
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	/**
	 * GAS
	 */

	// This is just a ref here, it lives on the PlayerState for player characters (and on the HoverTank for AI in the future)
	UPROPERTY()
	UHTAbilitySystemComponent* AbilitySystemComponent = nullptr;

	// Default attributes for a Tank on spawn, applied as an instant GameplayEffect
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> DefaultAttributes;

	// Effects that are always present on a Tank, like a constant shield recharge
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UHTGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|GAS", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* AbilityInputMappingContext;
	
	/** Ability Input Action - Q */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|GAS", meta = (AllowPrivateAccess = "true"))
	UInputAction* AbilityOneInputAction;

	/** Ability Input Action - E */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|GAS", meta = (AllowPrivateAccess = "true"))
	UInputAction* AbilityTwoInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> DeathEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> DamageEffect;

	// cached tags
	FGameplayTag DeadTag;

	bool bIsAbilitySystemComponentInputBound = false;

	virtual void InitializeAttributes();
	virtual void AddOngoingEffects();
	virtual void AddDefaultAbilities();

	void BindAbilitySystemComponentActions();

private:
	/**
	 * Actor components - Containers of logic
	 */
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	UHTTankMovementComponent* TankMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	UHTMovementReplicatorComponent* MovementReplicatorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UHTWeaponsComponent* WeaponsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
	UHTTankEffectsComponent* TankEffectsComponent = nullptr;

	/**
	 * Game components - Bits and pieces
	 */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ColliderMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TankBaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TankCannonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TankBarrelMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "2000.0"))
	float SpringArmLength = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1000.0"))
	float SpringArmZOffset = 200.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	URectLightComponent* TankLights = nullptr;

	/**
	 * Input mapping
	 */

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AltAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* StrafeAction;
	
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EBrakeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BoostAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ZoomInAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* NextWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PrevWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SelectWeapon01Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SelectWeapon02Action;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleLightsAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShowDebugAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SuicideAction;
	
	UPROPERTY(Replicated)
	bool bIsInputEnabled = true;
	
	/** Called for movement input */
	void MoveTriggered(const FInputActionValue& Value);
	void MoveCompleted();
	void MoveCanceled();

	void AltActionTriggered(const FInputActionValue& Value);
	void AltActionCompleted();

	void StrafeTriggered(const FInputActionValue& Value);
	void StrafeCompleted();

	/** Called for looking input */
	void LookTriggered(const FInputActionValue& Value);
	void LookCompleted();

	void EBrakeStarted();
	void EBrakeCompleted();

	void JumpTriggered();
	void JumpCompleted();

	void BoostTriggered();
	void BoostCompleted();
	
	void ShootStarted();

	void ZoomInActionStarted();
	void ZoomInActionCompleted();
	bool bIsZoomedIn = false;

	float DefaultCameraZoomFOV = 90;
	float ZoomedInCameraZoomFOV = 60;
	void HandleCameraZoom(float DeltaTime) const;

	void NextWeaponActionStarted(const FInputActionValue& Value);
	void PrevWeaponActionStarted(const FInputActionValue& Value);

	void SelectWeapon01ActionStarted();
	void SelectWeapon02ActionStarted();

	void ToggleLightsActionStarted();
	
	bool bShowDebug = false;
	void ShowDebugActionStarted();

	void SuicideActionStarted();

	/**
	 * GAS
	 */
	void AbilityOneStartedAction();
	void AbilityTwoStartedAction();
	
	/**
	 * Debug 
	 */
	void DebugDrawPlayerTitle();
};
