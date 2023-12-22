// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankMovementComponent.h"

#include "HoverTanks/HoverTank.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetMathLibrary.h"

UHoverTankMovementComponent::UHoverTankMovementComponent(): Throttle(0), Steering(0), LookUp(0), LookRight(0),
                                                            LastMove(),
															GroundTraceLocation(nullptr),
                                                            TankCannonMesh(nullptr),
                                                            TankBarrelMesh(nullptr),
                                                            LastCannonRotate()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UHoverTankMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	AHoverTank* HoverTank = Cast<AHoverTank>(GetOwner());
	if (HoverTank)
	{
		GroundTraceLocation = HoverTank->GetGroundTraceLocation();
		GroundTraceLocationOffset = HoverTank->GetGroundTraceLocationOffset();

		TankCannonMesh = HoverTank->GetTankCannonMesh();
		TankBarrelMesh = HoverTank->GetTankBarrelMesh();
	}
	
}

void UHoverTankMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	auto Owner = Cast<APawn>(GetOwner());
	if (Owner == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner is null"));
		return;
	}
	
	if (GetOwnerRole() == ROLE_AutonomousProxy || Owner->IsLocallyControlled())
	{
		LastMove = CreateMove(DeltaTime); // !! can crash on lever restart
		SimulateMove(LastMove);


		if (!TankCannonMesh || !TankBarrelMesh)
		{
			return;
		}

		Owner->AddControllerYawInput(LastCannonRotate.LookRight);
		Owner->AddControllerPitchInput(LastCannonRotate.LookUp);
		FRotator ControlRotation = Owner->GetControlRotation();
		
		LastCannonRotate = CreateCannonRotate(DeltaTime, ControlRotation);
		SimulateCannonRotate(LastCannonRotate);
	}
}

void UHoverTankMovementComponent::JumpTriggered()
{
	bIsJumping = true;
}

void UHoverTankMovementComponent::JumpCompleted()
{
	bIsJumping = false;
}

void UHoverTankMovementComponent::BoostTriggered()
{
	// handle boost reserve, CD
	bIsBoosting = true;
}

void UHoverTankMovementComponent::BoostCompleted()
{
	bIsBoosting = false;
}

/**
 * Movement Simulation covers
 *  - Throttle, acceleration, drag, rolling resistance
 *  - Updraft and gravity
 *  - Slopes and rotation along surface normals
 *  - Turning and Rotation
 *  - Cannon and barrel rotation 
 */
void UHoverTankMovementComponent::SimulateMove(FHoverTankMove Move)
{
	/**
	 * Initial Forces
	 */

	FVector ForceOnObject;
	FVector Acceleration;
	FVector VerticalForce;

	FVector GroundSurfaceNormal;
	float DistanceFromGround;

	GroundTraceLocation->SetWorldLocation(CalculateGroundTraceStartLocation());
	
	bool bIsGrounded = IsGrounded(GroundSurfaceNormal, DistanceFromGround);
	
	if (!IsInputEnabled())
	{
		Acceleration = FVector::ZeroVector;

		FVector Gravity = GetWorld()->GetGravityZ() / 100 * FVector(0, 0, 1);
		VerticalForce = Gravity * Move.DeltaTime;
	}
	else
	{
		float ThrottleValue = Move.bIsBoosting ? BoostThrottle : MaxThrottle;
		ForceOnObject = GetOwner()->GetActorForwardVector() * Move.Throttle * ThrottleValue;
	
		FVector AirResistance = CalculateAirResistance();
		FVector RollingResistance = CalculateRollingResistance(Move.bIsEBraking);

		ForceOnObject = ForceOnObject + AirResistance + RollingResistance;
		
		VerticalForce = CalculateVerticalForce(Move, DistanceFromGround);
		Acceleration = (ForceOnObject / Mass) * Move.DeltaTime;
	}

	Velocity = Velocity + Acceleration + VerticalForce;
	// clamp max speed
	Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);

	if (IsInputEnabled())
	{
		/**
		 * Turning and Rotation
		 */
		FRotator HorizontalRotation;
		FQuat RotationDelta;
		CalculateTurning(Move, HorizontalRotation, RotationDelta);

		FRotator AlignedRotation = CalculateSurfaceNormalRotation(GroundSurfaceNormal, GetOwner()->GetActorRightVector(), HorizontalRotation.Yaw);
		FRotator NewActorRotation = FMath::RInterpTo(HorizontalRotation, AlignedRotation, Move.DeltaTime, 2);

		GetOwner()->SetActorRotation(NewActorRotation);
		Velocity = RotationDelta.RotateVector(Velocity);
	}

	/**
	 * Move the Actor in 3d space
	 */
	FVector Translation = Velocity * Move.DeltaTime * 100; // * 100 to be in meters per seconds
	FHitResult HitResult;
	GetOwner()->AddActorWorldOffset(Translation, true, &HitResult);

	DebugDrawForwardAndVelocity();
	
	if (HitResult.IsValidBlockingHit() || HitResult.bStartPenetrating)
	{
		// FMath::GetReflectionVector ?
		FVector BounceVector = CalculateBounceVector(Velocity, HitResult.ImpactNormal);
		// DrawDebugDirectionalArrow(GetWorld(), HitResult.ImpactPoint, HitResult.ImpactPoint + BounceVector * 1000, 200, FColor::Red, false, 1, 0, 2);
		Velocity = BounceVector * Velocity.Size() / BounceDampening;
		// Velocity = FVector::ZeroVector;
	}
}

/**
 * Cannon Rotation Simulation covers the rotation of the TankCannon and TankBarrel meshes
 */
void UHoverTankMovementComponent::SimulateCannonRotate(const FHoverTankCannonRotate& CannonRotate)
{
	if (!IsInputEnabled())
	{
		return;
	}
	
	FRotator CannonRotation;
	CannonRotation.Pitch = 0;
	CannonRotation.Yaw = CannonRotate.ControlRotation.Yaw;
	CannonRotation.Roll = 0;
	TankCannonMesh->SetWorldRotation(CannonRotation);

	// try to rotate the TankBarrelMesh toward the ControlRotation
	FRotator BarrelRotation = TankBarrelMesh->GetComponentRotation();
	FRotator BarrelRotationTowardControlRotation = FMath::RInterpTo(BarrelRotation, CannonRotate.ControlRotation, CannonRotate.DeltaTime, 10);
	BarrelRotationTowardControlRotation.Pitch = BarrelRotationTowardControlRotation.Pitch + .25f; // zeroing 
	BarrelRotationTowardControlRotation.Yaw = CannonRotate.ControlRotation.Yaw;
	BarrelRotationTowardControlRotation.Roll = 0;
	
	BarrelRotationTowardControlRotation.Pitch = FMath::Clamp(BarrelRotationTowardControlRotation.Pitch, -10.0f, 15.0f);
	
	TankBarrelMesh->SetWorldRotation(BarrelRotationTowardControlRotation);
}

FHoverTankMove UHoverTankMovementComponent::CreateMove(float DeltaTime)
{
	FHoverTankMove Move;
	Move.DeltaTime = DeltaTime;
	Move.Throttle = Throttle;
	Move.Steering = Steering;
	Move.bIsEBraking = bIsEBraking;
	Move.bIsJumping = bIsJumping;
	Move.bIsBoosting = bIsBoosting;
	Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	return Move;
}

FHoverTankCannonRotate UHoverTankMovementComponent::CreateCannonRotate(float DeltaTime, const FRotator& ControlRotation)
{
	FHoverTankCannonRotate CannonRotate;
	CannonRotate.DeltaTime = DeltaTime;
	CannonRotate.LookUp = LookUp;
	CannonRotate.LookRight = LookRight;
	CannonRotate.ControlRotation = ControlRotation;

	return CannonRotate;
}

/**
 * Steering turns the Actor.
 * Velocity is rotated by the Steering angle. The lower the speed is, the more turning is applied to the Velocity.
 * More speed means more drift.
 */
void UHoverTankMovementComponent::CalculateTurning(const FHoverTankMove& Move, FRotator &HorizontalRotation, FQuat &RotationDelta)
{
	// Rotate Actor based on Steering
	HorizontalRotation = GetOwner()->GetActorRotation();
	float RotationAngle = BaseTurnRate * Move.Steering * Move.DeltaTime;
	HorizontalRotation.Yaw += RotationAngle;

	// Rotate Velocity based on Steering, and Drift Ratio
	// As velocity appraoches 0, DriftDivider should approach 1, and as Velocity approaches 20 DriftDivider should approach 2
	float DriftDivider = MaxDriftRatio > KINDA_SMALL_NUMBER
		? 1 + (Velocity.Size() / 20) * MaxDriftRatio
		: 1;
	
	RotationAngle = FMath::DegreesToRadians(RotationAngle) / DriftDivider;
	RotationDelta = FQuat(GetOwner()->GetActorUpVector(), RotationAngle);
}

FRotator UHoverTankMovementComponent::CalculateSurfaceNormalRotation(const FVector& GroundSurfaceNormal, FVector RightVector, float ActorYawRotation)
{
	float OutSlopePitchDegreeAngle;
	float OutSlopeRollDegreeAngle;
	FVector UpVector = FVector(0, 0, 1);
	
	UKismetMathLibrary::GetSlopeDegreeAngles(RightVector.GetSafeNormal(), GroundSurfaceNormal.GetSafeNormal(), UpVector, OutSlopePitchDegreeAngle, OutSlopeRollDegreeAngle);

	OutSlopeRollDegreeAngle = -OutSlopeRollDegreeAngle;
	OutSlopePitchDegreeAngle = FMath::Clamp(OutSlopePitchDegreeAngle, -20.0f, 20.0f);
	
	FRotator AlignedRotation = FRotator(OutSlopePitchDegreeAngle, ActorYawRotation, OutSlopeRollDegreeAngle);
	
	// DrawDebugBox(GetWorld(), GetOwner()->GetActorLocation() + FVector(0, 0, 500), FVector(100, 100, 100), AlignedRotation.Quaternion(), FColor::Purple, false, 0, 0, 2);
	// UE_LOG(LogTemp, Warning, TEXT("Actor Rotation: %s AlignedRotation: %s"), *GetOwner()->GetActorRotation().ToString(), *AlignedRotation.ToString());

	return AlignedRotation;
}

FVector UHoverTankMovementComponent::CalculateAirResistance()
{
	return Velocity.GetSafeNormal() * -1 * Velocity.SizeSquared() * DragCoefficient;
}

FVector UHoverTankMovementComponent::CalculateRollingResistance(bool InIsEBraking)
{
	float AccelerationDueToGravity = GetWorld()->GetGravityZ() / 100; // 1 to 100 to be in meters per seconds
	
	float NormalForce = Mass * AccelerationDueToGravity;
	
	if (InIsEBraking)
	{
		NormalForce *= 20;
	}

	// UE_LOG(LogTemp, Warning, TEXT("Normal Force: %f"), NormalForce);

	return Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}

FVector UHoverTankMovementComponent::CalculateBounceVector(const FVector& InVelocity, const FVector& WallNormal)
{
	// Ensure that the incoming velocity and wall normal are normalized
	FVector NormalizedVelocity = InVelocity.GetSafeNormal();
	FVector NormalizedWallNormal = WallNormal.GetSafeNormal();

	// Calculate the reflection vector using the formula: R = I - 2 * (I dot N) * N
	FVector BounceVector = NormalizedVelocity - 2.0f * FVector::DotProduct(NormalizedVelocity, NormalizedWallNormal) * NormalizedWallNormal;

	return BounceVector;
}

FVector UHoverTankMovementComponent::CalculateVerticalForce(const FHoverTankMove& Move, float DistanceFromGround)
{
	FVector Gravity = GetWorld()->GetGravityZ() / 100 * FVector(0, 0, 1);

	FVector VerticalForce;
	if (DistanceFromGround < DesiredFloatHeight * 2)
	{
		float UpDraftMultiplier = 1 - DistanceFromGround / DesiredFloatHeight;
		float UpVectorMagnitude = UpDraftMultiplier * -Gravity.Z; // * HoverBounceMultiplier
	
		VerticalForce = FVector(0, 0, 1) * UpVectorMagnitude;
		// FVector DownForce = VerticalForce - Gravity;
		// UE_LOG(LogTemp, Warning, TEXT("DST: %f, GRV: %f UpDraft: %f, VForce %f, m: %f"), DistanceFromGround, Gravity.Z, DownForce.Size(), VerticalForce.Z, UpDraftMultiplier);
	}
	else
	{
		VerticalForce = Gravity;
	}

	if (Move.bIsJumping)
	{
		VerticalForce += FVector(0, 0, 1) * 5;
		// DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation() + FVector(10, 0, 150), GetOwner()->GetActorLocation() + FVector(10, 0, 250), 10, FColor::Red, false, 0, 0, 8);
	}

	if (Move.bIsEBraking)
	{
		VerticalForce -= FVector(0, 0, 1) * 5;
		// DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation() + FVector(-10, 0, 250), GetOwner()->GetActorLocation() + FVector(-10, 0, 150), 10, FColor::Blue, false, 0, 0, 8);
	}

	return VerticalForce * Move.DeltaTime;
}

/**
 * Move the GroundTraceStart component along the Velocity. As Velocity magnitude increases, the GroundTraceStart
 * component should move further ahead, maximized at MaxSpeed. As Velocity magnitude decreases, the GroundTraceStart
 * component should closer to it's starting location. This way the HoverTank will rotate to match a slope sooner.
 */
FVector UHoverTankMovementComponent::CalculateGroundTraceStartLocation()
{
	FVector GroundTraceOriginLocation = GetOwner()->GetActorLocation() + GroundTraceLocationOffset;
	FVector NormalizedVelocity = Velocity.GetSafeNormal();
	FVector NormalizedVelocityXZ = FVector(NormalizedVelocity.X, NormalizedVelocity.Y, 0);
	FVector GroundTraceTargetLocation;
	
	float OffsetMaxMagnitude = 500;

	// Set the OffsetMagnitude to be a function of the Velocity magnitude. When we are at a standstill it should be 0. When we reached MaxSpeed it should be MaxOffsetMagnitude
	float OffsetMagnitude = OffsetMaxMagnitude * (Velocity.Size() / MaxSpeed);

	// UE_LOG(LogTemp, Warning, TEXT("OffsetMagnitude: %f"), OffsetMagnitude);
	
	GroundTraceTargetLocation = GroundTraceOriginLocation + NormalizedVelocityXZ * OffsetMagnitude;

	return GroundTraceTargetLocation;
}

bool UHoverTankMovementComponent::IsGrounded(FVector &GroundSurfaceNormal, float &DistanceFromGround)
{
	if (GetOwner() == nullptr)
	{
		return false;
	}

	float GroundCheckDistance = 20000.f;
	float GroundNormalThreshold = 0.7f;
	
	// Set up the parameters for the line trace
	// FVector StartLocation = GetOwner()->GetActorLocation();
	FVector StartLocation = GroundTraceLocation->GetComponentLocation();

	// draw a debug cube at the Startlocation
	DrawDebugBox(GetWorld(), StartLocation, FVector(100, 100, 100), FColor::Green, false, 0, 0, 2);
	
	// StartLocation.Z -= 75;
	FVector EndLocation = StartLocation - FVector(0, 0, GroundCheckDistance);

	// Perform a line trace to check for the ground
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());

	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green, false, 1, 0, 1);

	GroundSurfaceNormal = FVector(0, 0, 1);
	
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams))
	{
		GroundSurfaceNormal = HitResult.ImpactNormal.GetSafeNormal();
		DistanceFromGround = HitResult.Distance;

		// Check if the hit surface is walkable and is close enough
		if (HitResult.ImpactNormal.Z >= GroundNormalThreshold && HitResult.Distance <= 200)
		{
			// DrawDebugLine(GetWorld(), HitResult.Location, HitResult.Location + GroundSurfaceNormal * 500 , FColor::Red, false, 1, 0, 2);
			return true;
		}
	}

	return false;
}

bool UHoverTankMovementComponent::IsInputEnabled()
{
	AHoverTank* HoverTank = Cast<AHoverTank>(GetOwner());
	if (HoverTank)
	{
		return HoverTank->IsInputEnabled();
	}

	return false;
}

void UHoverTankMovementComponent::DebugDrawForwardAndVelocity() const
{
	DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + Velocity.GetSafeNormal() * 1000, 10, FColor::Purple, false, 0, 0, 4);
	DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 1000, 10, FColor::Green, false, 0, 0, 4);
}
