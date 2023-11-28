// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HoverTankMovementComponent.generated.h"

class UBoxComponent;

USTRUCT()
struct FHoverTankMove
{
	GENERATED_BODY()

	UPROPERTY()
	float Throttle;
	UPROPERTY()
	float Steering;
	UPROPERTY()
	bool bIsEBraking;
	UPROPERTY()
	bool bJumpOnNextTick;

	UPROPERTY()
	float DeltaTime;
	UPROPERTY()
	float Time;

	bool IsValid() const
	{
		return FMath::Abs(Throttle) <= 1 && FMath::Abs(Steering) <= 1;
	}
};

USTRUCT()
struct FHoverTankCannonRotate
{
	GENERATED_BODY()

	UPROPERTY()
	float LookUp;

	UPROPERTY();
	float LookRight;

	UPROPERTY()
	FRotator ControlRotation;

	UPROPERTY()
	float DeltaTime;

	bool IsValid() const
	{
		return FMath::Abs(LookUp) <= 1 && FMath::Abs(LookRight) <= 1;
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOVERTANKS_API UHoverTankMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHoverTankMovementComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetThrottle(float InThrottle) { Throttle = InThrottle; }
	void SetSteering(float InSteering) { Steering = InSteering; }
	void SetIsEBraking(bool bInIsEBraking) { bIsEBraking = bInIsEBraking; }

	void JumpPressed();

	void SimulateMove(FHoverTankMove Move);
	FHoverTankMove GetLastMove() { return LastMove; }

	FVector GetVelocity() { return Velocity; }
	void SetVelocity(FVector InVelocity) { Velocity = InVelocity; }

	void SetLookUp(float InLookUp) { LookUp = InLookUp; }
	void SetLookRight(float InLookRight) { LookRight = InLookRight; }

	void SimulateCannonRotate(const FHoverTankCannonRotate& CannonRotate);
	FHoverTankCannonRotate GetLastCannonRotate() { return LastCannonRotate; }

	UStaticMeshComponent* GetTankCannonMesh() { return TankCannonMesh; }
	UStaticMeshComponent* GetTankBarrelMesh() { return TankBarrelMesh; }
	
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	/** In Newtons. The max driving force */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MaxThrottle = 40000;

	/** Tank turn rate in degrees per second */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate = 75;

	/** The mass of the tank (kg). */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Mass = 5000;

	/** Higher values means more drag. kg/m */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPricateAccess = "true"))
	float DragCoefficient = 32;

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPricateAccess = "true"))
	float RollingResistanceCoefficient = 0.03f;

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPricateAccess = "true", ClampMin="0.0", ClampMax="1.0"))
	float MaxDriftRatio = 1.f;

	/** Higher value means more loss of momentum on collision */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPricateAccess = "true", ClampMin="1.0", ClampMax="3.0"))
	float BounceDampening = 2.0f;

	float Throttle;
	float Steering;
	bool bIsEBraking = false;

	bool bJumpOnNextTick = false;

	/** Cannon turn rate in degrees per second, looking left and right */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CannonTurnRate = 90;

	/** Barrel pitch rate in degrees per second, looking up and down */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float BarrelPitchRate = 90;

	float LookUp;
	float LookRight;
	
	FVector Velocity;

	FHoverTankMove LastMove;
	FHoverTankMove CreateMove(float DeltaTime);

	/**
	 * Tank Cannon and Barrel Rotations
	 */
	UPROPERTY()
	UStaticMeshComponent* TankCannonMesh;

	UPROPERTY()
	UStaticMeshComponent* TankBarrelMesh;

	FHoverTankCannonRotate LastCannonRotate;
	FHoverTankCannonRotate CreateCannonRotate(float DeltaTime, const FRotator& ControlRotation);

	/**
	 * Helper Functions
	 */
	FVector CalculateAirResistance();
	FVector CalculateRollingResistance(bool InIsEBraking);
	void CalculateTurning(const FHoverTankMove& Move, FRotator &HorizontalRotation, FQuat &RotationDelta);
	FRotator CalculateSurfaceNormalRotation(const FVector& GroundSurfaceNormal, float DeltaTime);
	FVector CalculateBounceVector(const FVector& InVelocity, const FVector& WallNormal);

	bool IsGrounded(FVector &GroundSurfaceNormal, float &DistanceFromGround);

	/** In Newtons. */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MaxUpDraftForce = 9.8f;

	FVector CalculateDownForce(const FHoverTankMove& Move, bool bIsGrounded, float DistanceFromGround);

	/**
	 * Debug
	 */
	void DebugDrawForwardAndVelocity() const;
};
