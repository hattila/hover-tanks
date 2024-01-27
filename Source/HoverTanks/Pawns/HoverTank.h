// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HasTeamColors.h"
#include "HoverTank.generated.h"

class UWeaponsComponent;
class UHoverTankEffectsComponent;
class UHoverTankMovementComponent;
class UMovementReplicatorComponent;
class UHealthComponent;

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTankHealthChange, float, Health, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwitched, int32, NewWeapon);

UCLASS()
class HOVERTANKS_API AHoverTank : public APawn, public IHasTeamColors
{
	GENERATED_BODY()
	
public:
	AHoverTank();
	
	UPROPERTY(BlueprintAssignable, Category = "CustomEvents")
	FOnTankDeath OnTankDeath;

	UPROPERTY(BlueprintAssignable, Category = "CustomEvents")
	FOnTankHealthChange OnTankHealthChange;

	UPROPERTY(BlueprintAssignable, Category = "CustomEvents")
	FOnWeaponSwitched OnWeaponSwitched;

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	virtual void OnRep_PlayerState() override;

	UFUNCTION(BlueprintPure)
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

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
	bool IsInputEnabled() const { return bIsInputEnabled; }

	void SetInputEnabled(const bool bNewInputEnabled) { bIsInputEnabled = bNewInputEnabled; }

	bool GetShowDebug() const { return bShowDebug; }

	UFUNCTION(Client, Unreliable)
	void ClientBroadcastOnTankDeath();

	FHitResult FindTargetAtCrosshair() const;
	
	UWeaponsComponent* GetWeaponsComponent() const { return WeaponsComponent; }
	UHoverTankEffectsComponent* GetEffectsComponent() const { return HoverTankEffectsComponent; }

	//~ Begin IHasTeamColors interface
	virtual void ApplyTeamColors(UTeamDataAsset* TeamDataAsset) override;
	//~ End of IHasTeamColors interface

protected:
	virtual void BeginPlay() override;

	void InitPlayer();
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

private:
	/**
	 * Actor components
	 */
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	UHoverTankMovementComponent* HoverTankMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	UMovementReplicatorComponent* MovementReplicatorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UWeaponsComponent* WeaponsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
	UHoverTankEffectsComponent* HoverTankEffectsComponent = nullptr;

	/**
	 * Game components
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
	UInputAction* ToggleLightsAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShowDebugAction;
	
	UPROPERTY(Replicated)
	bool bIsInputEnabled = true;
	
	/** Called for movement input */
	void MoveTriggered(const FInputActionValue& Value);
	void MoveCompleted();

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

	void ToggleLightsActionStarted();
	
	bool bShowDebug = false;
	void ShowDebugActionStarted();
	
	/**
	 * Debug 
	 */
	void DebugDrawPlayerTitle();
};
