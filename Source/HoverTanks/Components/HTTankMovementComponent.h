// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HTTankMovementComponent.generated.h"

USTRUCT()
struct FHoverTankMove
{
	GENERATED_BODY()

	UPROPERTY()
	float Throttle;
	UPROPERTY()
	float SideStrafeThrottle;
	UPROPERTY()
	float Steering;
	UPROPERTY()
	bool bIsEBraking;
	UPROPERTY()
	bool bIsJumping;
	UPROPERTY()
	bool bIsBoosting;
	UPROPERTY()
	FVector DirectionalLaunchVelocity; // probably should be a normalized vector, with a separate magnitude value for easier validation

	UPROPERTY()
	float DeltaTime;
	UPROPERTY()
	float Time;

	bool IsValid() const
	{
		return FMath::Abs(Throttle) <= 1 && FMath::Abs(Steering) <= 1 && FMath::Abs(SideStrafeThrottle) <= 1;
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
class HOVERTANKS_API UHTTankMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHTTankMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetThrottle(const float InThrottle) { Throttle = InThrottle; }
	void SetSideStrafeThrottle(const float InSideStrafeThrottle) { SideStrafeThrottle = InSideStrafeThrottle; }
	void SetSteering(const float InSteering) { Steering = InSteering; }
	void SetIsEBraking(const bool bInIsEBraking) { bIsEBraking = bInIsEBraking; }
	void SetDirectionalLaunchVelocity(const FVector& InVector) { DirectionalLaunchVelocity = InVector; };

	void JumpTriggered();
	void JumpCompleted();

	void BoostTriggered();
	void BoostCompleted();
	
	void SimulateMove(FHoverTankMove Move);
	FHoverTankMove GetLastMove() const { return LastMove; }

	FVector GetVelocity() const { return Velocity; }
	void SetVelocity(const FVector& InVelocity) { Velocity = InVelocity; }

	void SetLookUp(const float InLookUp) { LookUp = InLookUp; }
	void SetLookRight(const float InLookRight) { LookRight = InLookRight; }

	void SimulateCannonRotate(const FHoverTankCannonRotate& CannonRotate);
	FHoverTankCannonRotate GetLastCannonRotate() const { return LastCannonRotate; }

	UStaticMeshComponent* GetTankCannonMesh() const { return TankCannonMesh; }
	UStaticMeshComponent* GetTankBarrelMesh() const { return TankBarrelMesh; }

	bool IsGrounded(FVector &GroundSurfaceNormal, float &DistanceFromGround, FHitResult &HitResult) const;

protected:
	virtual void BeginPlay() override;

private:
	/** In Newtons. The driving force when accelerating */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MaxThrottle = 30000;

	/** In Newtons. The max driving force when boosting */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float BoostThrottle = 90000;

	/** In Metres per second. The max speed */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MaxSpeed = 25;

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
	float MaxDriftRatio = 0.5f;

	/** Value increases or decreases forward momentum */
	float Throttle = 0;

	/** Value increases or decreases side-strafe momentum */
	float SideStrafeThrottle = 0;

	/** Value increases or decreases turning */
	float Steering = 0;

	bool bIsEBraking = false;
	bool bIsJumping = false;
	bool bIsBoosting = false;

	float LookUp = 0;
	float LookRight = 0;
	
	FVector Velocity;

	/**
	 * This is used to pass in a value from JumpPads or other objects that should launch the tank in a specific direction.
	 */
	FVector DirectionalLaunchVelocity = FVector::ZeroVector;

	/** In Unreal Units (Centimeters) */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float DesiredFloatHeight = 100;

	FHoverTankMove LastMove;
	FHoverTankMove CreateMove(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* GroundTraceLocation = nullptr;

	// The amount of offset from the center off mass toward the ground, where the ground trace should start.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	FVector GroundTraceLocationOffset = FVector(0.f, 0.f, -75.f);

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float GroundTraceLocationOffsetMaxMagnitude = 500;
	
	/**
	 * Tank Cannon and Barrel Rotations
	 */
	UPROPERTY()
	UStaticMeshComponent* TankCannonMesh = nullptr;

	UPROPERTY()
	UStaticMeshComponent* TankBarrelMesh = nullptr;

	FHoverTankCannonRotate LastCannonRotate;
	FHoverTankCannonRotate CreateCannonRotate(float DeltaTime, const FRotator& ControlRotation);

	/**
	 * Helper Functions
	 */
	FVector CalculateAirResistance();
	FVector CalculateRollingResistance(bool InIsEBraking);
	void CalculateTurning(const FHoverTankMove& Move, FRotator &HorizontalRotation, FQuat &RotationDelta);
	FRotator CalculateSurfaceNormalRotation(const bool bIsGrounded, const FVector& GroundSurfaceNormal, FVector RightVector, float ActorYawRotation);
	FVector CalculateBounceVector(const FVector& InVelocity, const FVector& WallNormal);
	FVector CalculateVerticalForce(const FHoverTankMove& Move, float DistanceFromGround, bool bIsGrounded);
	FVector CalculateGroundTraceStartLocation() const;
	
	// bool IsGrounded(FVector &GroundSurfaceNormal, float &DistanceFromGround) const;
	bool IsInputEnabled();
	bool IsTankDead() const;

	/**
	 * Debug
	 */
	bool ShowDebug() const;
	void DebugDrawForwardAndVelocity() const;
};
