// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HoverTank.generated.h"

class UHoverTankHUDWidget;
class USphereComponent;
class UWeaponsComponent;
class UHealthComponent;
class UHoverTankMovementComponent;
class UMovementReplicatorComponent;

struct FInputActionValue;
class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTankDeath);

UCLASS()
class HOVERTANKS_API AHoverTank : public APawn
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHoverTank();
	
	UPROPERTY(BlueprintAssignable, Category = "CustomEvents")
	FOnTankDeath OnTankDeath;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// create definition of the standard input binding method
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure)
	USceneComponent* GetGroundTraceLocation() const { return GroundTraceLocation; }

	FVector GetGroundTraceLocationOffset() const { return GroundTraceLocationOffset; }
	
	UFUNCTION(BlueprintPure)
	UStaticMeshComponent* GetTankCannonMesh() { return TankCannonMesh; }

	UFUNCTION(BlueprintPure)
	UStaticMeshComponent* GetTankBarrelMesh() { return TankBarrelMesh; }

	void OnDeath();
	bool IsDead() const;
	bool IsInputEnabled() const { return bIsInputEnabled; }

	void SetInputEnabled(const bool bNewInputEnabled) { bIsInputEnabled = bNewInputEnabled; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	UHoverTankMovementComponent* HoverTankMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	UMovementReplicatorComponent* MovementReplicatorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UWeaponsComponent* WeaponsComponent;

	// create a Box collider
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ColliderMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* GroundTraceLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	FVector GroundTraceLocationOffset = FVector(0.f, 0.f, -75.f);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FCollisionProfileName CollisionProfile;
	
	// create a static mesh component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TankBaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TankCannonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TankBarrelMesh;
	
	// create a spring arm component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "2000.0"))
	float SpringArmLength = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1000.0"))
	float SpringArmZOffset = 200.f;
	
	// create a camera component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

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

	/**
	 * Debug 
	 */
	void DebugDrawPlayerTitle();
	void DebugDrawSphereAsCrosshair() const;
};
