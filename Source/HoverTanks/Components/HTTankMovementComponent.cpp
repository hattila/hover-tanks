// Fill out your copyright notice in the Description page of Project Settings.


#include "HTTankMovementComponent.h"

#include "HoverTanks/Pawns/HoverTank/HTHoverTank.h"

#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetMathLibrary.h"

UHTTankMovementComponent::UHTTankMovementComponent(): LastMove(), LastCannonRotate()
{
	PrimaryComponentTick.bCanEverTick = true;

	GroundTraceLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Ground Trace Start"));
	GroundTraceLocation->SetRelativeLocation(GroundTraceLocationOffset);
}

void UHTTankMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	AHTHoverTank* HoverTank = Cast<AHTHoverTank>(GetOwner());
	if (HoverTank)
	{
		TankCannonMesh = HoverTank->GetTankCannonMesh();
		TankBarrelMesh = HoverTank->GetTankBarrelMesh();

		GroundTraceLocation->AttachToComponent(HoverTank->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}
	
}

void UHTTankMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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
		LastMove = CreateMove(DeltaTime);
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

void UHTTankMovementComponent::JumpTriggered()
{
	bIsJumping = true;
}

void UHTTankMovementComponent::JumpCompleted()
{
	bIsJumping = false;
}

void UHTTankMovementComponent::BoostTriggered()
{
	// handle boost reserve, CD
	bIsBoosting = true;
}

void UHTTankMovementComponent::BoostCompleted()
{
	bIsBoosting = false;
}

/**
 * Movement Simulation covers
 *  - Throttle, acceleration, drag, rolling resistance
 *  - Updraft and gravity
 *  - Bounce angle and dampening
 *  - Slopes and rotation along surface normals
 *  - Turning and Rotation
 */
void UHTTankMovementComponent::SimulateMove(FHoverTankMove Move)
{
	/**
	 * Initial Forces
	 */

	FVector ForceOnObject;
	FVector Acceleration;
	FVector VerticalForce;

	GroundTraceLocation->SetWorldLocation(CalculateGroundTraceStartLocation());

	FVector GroundSurfaceNormal;
	float DistanceFromGround;
	FHitResult GroundTranceHitResult;

	bool bIsGrounded = IsGrounded(GroundSurfaceNormal, DistanceFromGround, GroundTranceHitResult);
	
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
		
		VerticalForce = CalculateVerticalForce(Move, DistanceFromGround, bIsGrounded);
		Acceleration = (ForceOnObject / Mass) * Move.DeltaTime;
	}

	Velocity = Velocity + Acceleration + VerticalForce;

	/**
	 * In order to never really hit the ground, Velocity.Z should be clamped to 0, if the ground is closer than 50 units
	 * Moving around dunes is much smoother than bouncing around on the ground.
	 */
	if (DistanceFromGround < 50 && Velocity.Z < 0)
	{
		Velocity.Z = 0;
	}
	
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

		FRotator AlignedRotation = CalculateSurfaceNormalRotation(bIsGrounded, GroundSurfaceNormal, GetOwner()->GetActorRightVector(), HorizontalRotation.Yaw);
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

	if (ShowDebug())
	{
		DebugDrawForwardAndVelocity();	
	}
	
	if (HitResult.IsValidBlockingHit() || HitResult.bStartPenetrating)
	{
		// FMath::GetReflectionVector ?
		FVector BounceVector = CalculateBounceVector(Velocity, HitResult.ImpactNormal);
		// DrawDebugDirectionalArrow(GetWorld(), HitResult.ImpactPoint, HitResult.ImpactPoint + BounceVector * 1000, 200, FColor::Red, false, 1, 0, 2);

		float BounceDampening = .5;
		if (!IsTankDead())
		{
			float ImpactSeverity = FMath::Abs(FVector::DotProduct(BounceVector, HitResult.ImpactNormal));
			BounceDampening = 1 - ImpactSeverity + .2f;
			BounceDampening = FMath::Clamp(BounceDampening, .2f, 0.8f);
		}

		// UE_LOG(LogTemp, Warning, TEXT("ImpactSeverity: %f, BounceDampening: %f"), ImpactSeverity, BounceDampening);
		Velocity = BounceVector * Velocity.Size() * BounceDampening;
		// Velocity = FVector::ZeroVector;
	}
}

/**
 * Cannon Rotation Simulation covers the rotation of the TankCannon and TankBarrel meshes
 */
void UHTTankMovementComponent::SimulateCannonRotate(const FHoverTankCannonRotate& CannonRotate)
{
	if (!IsInputEnabled())
	{
		return;
	}

	// log out CannonRotate ControlRotation, and Pitch
	// UE_LOG(LogTemp, Warning, TEXT("CannonRotate ControlRotation: %s, Pitch: %f"), *CannonRotate.ControlRotation.ToString(), CannonRotate.ControlRotation.Pitch);
	
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
	
	BarrelRotationTowardControlRotation.Pitch = FMath::Clamp(BarrelRotationTowardControlRotation.Pitch, -20.0f, 25.0f);

	// log out calculated Pitch
	// UE_LOG(LogTemp, Warning, TEXT("BarrelRotationTowardControlRotation Pitch: %f"), BarrelRotationTowardControlRotation.Pitch);
	
	TankBarrelMesh->SetWorldRotation(BarrelRotationTowardControlRotation);
}

FHoverTankMove UHTTankMovementComponent::CreateMove(float DeltaTime)
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

FHoverTankCannonRotate UHTTankMovementComponent::CreateCannonRotate(float DeltaTime, const FRotator& ControlRotation)
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
void UHTTankMovementComponent::CalculateTurning(const FHoverTankMove& Move, FRotator &HorizontalRotation, FQuat &RotationDelta)
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

FRotator UHTTankMovementComponent::CalculateSurfaceNormalRotation(const bool bIsGrounded, const FVector& GroundSurfaceNormal, FVector RightVector, float ActorYawRotation)
{
	if (!bIsGrounded)
	{
		return FRotator(0, ActorYawRotation, 0);
	}
	
	float OutSlopePitchDegreeAngle;
	float OutSlopeRollDegreeAngle;
	FVector UpVector = FVector(0, 0, 1);
	
	UKismetMathLibrary::GetSlopeDegreeAngles(RightVector.GetSafeNormal(), GroundSurfaceNormal.GetSafeNormal(), UpVector, OutSlopePitchDegreeAngle, OutSlopeRollDegreeAngle);

	OutSlopeRollDegreeAngle = -OutSlopeRollDegreeAngle;
	OutSlopePitchDegreeAngle = FMath::Clamp(OutSlopePitchDegreeAngle, -30.0f, 30.0f);
	
	FRotator AlignedRotation = FRotator(OutSlopePitchDegreeAngle, ActorYawRotation, OutSlopeRollDegreeAngle);

	if (ShowDebug())
	{
		DrawDebugBox(GetWorld(), GetOwner()->GetActorLocation() + FVector(0, 0, 300), FVector(100, 100, 20), AlignedRotation.Quaternion(), FColor::Purple, false, 0, 0, 2);	
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("Actor Rotation: %s AlignedRotation: %s"), *GetOwner()->GetActorRotation().ToString(), *AlignedRotation.ToString());

	return AlignedRotation;
}

FVector UHTTankMovementComponent::CalculateAirResistance()
{
	return Velocity.GetSafeNormal() * -1 * Velocity.SizeSquared() * DragCoefficient;
}

FVector UHTTankMovementComponent::CalculateRollingResistance(bool InIsEBraking)
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

FVector UHTTankMovementComponent::CalculateBounceVector(const FVector& InVelocity, const FVector& WallNormal)
{
	// Ensure that the incoming velocity and wall normal are normalized
	const FVector NormalizedVelocity = InVelocity.GetSafeNormal();
	const FVector NormalizedWallNormal = WallNormal.GetSafeNormal();

	// Calculate the reflection vector using the formula: R = I - 2 * (I dot N) * N
	const FVector BounceVector = NormalizedVelocity - 2.0f * FVector::DotProduct(NormalizedVelocity, NormalizedWallNormal) * NormalizedWallNormal;

	return BounceVector;
}

FVector UHTTankMovementComponent::CalculateVerticalForce(const FHoverTankMove& Move, float DistanceFromGround, bool bIsGrounded)
{
	const FVector Gravity = GetWorld()->GetGravityZ() / 100 * FVector(0, 0, 1); // m/s^2

	FVector VerticalForce;
	if (bIsGrounded)
	{
		DistanceFromGround -= 50; // skew the distance, so the UpDraftMultiplier is 1 when we are 50 units above the ground
		float UpDraftMultiplier = 1 - DistanceFromGround / (DesiredFloatHeight);

		if (UpDraftMultiplier > 0)
		{
			// the closer to the ground we get, the higher the UpDraftMultiplier, square it for greater effect
			UpDraftMultiplier = FMath::Square(UpDraftMultiplier);
		}

		const float UpVectorMagnitude = UpDraftMultiplier * -Gravity.Z;
		
		VerticalForce = FVector(0, 0, 1) * UpVectorMagnitude;
		// FVector DownForce = VerticalForce - Gravity;
		// UE_LOG(LogTemp, Warning, TEXT("DST: %f, VForce %f \n Multiplier: %f UpDraft: %f, \n GRV: %f \n\n\n"), DistanceFromGround, VerticalForce.Z, UpDraftMultiplier, DownForce.Size(), Gravity.Z);

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
	}
	else
	{
		VerticalForce = Gravity;
		// UE_LOG(LogTemp, Warning, TEXT("Not Grounded, Falling. VForce %f, GRV: %f"), VerticalForce.Z, Gravity.Z);
	}

	return VerticalForce * Move.DeltaTime;
}

/**
 * Move the GroundTraceStart component along the Velocity. As Velocity magnitude increases, the GroundTraceStart
 * component should move further ahead, maximized at MaxSpeed. As Velocity magnitude decreases, the GroundTraceStart
 * component should get closer to it's starting location. This way the HoverTank will rotate to match a slope sooner.
 */
FVector UHTTankMovementComponent::CalculateGroundTraceStartLocation() const
{
	const FVector GroundTraceOriginLocation = GetOwner()->GetActorLocation() + GroundTraceLocationOffset;
	const FVector NormalizedVelocity = Velocity.GetSafeNormal();
	const FVector NormalizedVelocityXZ = FVector(NormalizedVelocity.X, NormalizedVelocity.Y, 0);

	const float OffsetMaxMagnitude = 500;

	// Set the OffsetMagnitude to be a function of the Velocity magnitude. When we are at a standstill it should be 0. When we reached MaxSpeed it should be MaxOffsetMagnitude
	const float OffsetMagnitude = OffsetMaxMagnitude * (Velocity.Size() / MaxSpeed);
	const FVector GroundTraceTargetLocation = GroundTraceOriginLocation + NormalizedVelocityXZ * OffsetMagnitude;

	return GroundTraceTargetLocation;
}

bool UHTTankMovementComponent::IsGrounded(FVector &GroundSurfaceNormal, float &DistanceFromGround, FHitResult &HitResult) const
{
	if (GetOwner() == nullptr)
	{
		return false;
	}

	float GroundCheckDistance = 20000.f;
	float GroundNormalThreshold = 0.7f;
	FVector TraceStartLocation = GroundTraceLocation->GetComponentLocation();

	if (ShowDebug())
	{
		DrawDebugBox(GetWorld(), TraceStartLocation, FVector(100, 100, 100), FColor::Green, false, 0, 0, 2);
	}

	FVector EndLocation = TraceStartLocation - FVector(0, 0, GroundCheckDistance);
	// FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());

	if (ShowDebug())
	{
		DrawDebugLine(GetWorld(), TraceStartLocation, EndLocation, FColor::Green, false, 1, 0, 1);
	}

	GroundSurfaceNormal = FVector(0, 0, 1);
	
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStartLocation, EndLocation, ECC_Visibility, CollisionParams))
	{
		GroundSurfaceNormal = HitResult.ImpactNormal.GetSafeNormal();
		DistanceFromGround = HitResult.Distance;

		// Check if the hit surface is walkable and is close enough
		if (HitResult.ImpactNormal.Z >= GroundNormalThreshold && HitResult.Distance <= 400)
		{
			// DrawDebugLine(GetWorld(), HitResult.Location, HitResult.Location + GroundSurfaceNormal * 500 , FColor::Red, false, 1, 0, 2);
			return true;
		}
	}

	return false;
}

bool UHTTankMovementComponent::IsInputEnabled()
{
	AHTHoverTank* HoverTank = Cast<AHTHoverTank>(GetOwner());
	if (HoverTank)
	{
		return HoverTank->IsInputEnabled();
	}

	return false;
}

bool UHTTankMovementComponent::IsTankDead() const
{
	AHTHoverTank* HoverTank = Cast<AHTHoverTank>(GetOwner());
	if (HoverTank)
	{
		return HoverTank->IsDead();
	}

	return false;
}

bool UHTTankMovementComponent::ShowDebug() const
{
	if (const AHTHoverTank* HoverTank = Cast<AHTHoverTank>(GetOwner()))
	{
		return HoverTank->GetShowDebug();
	}

	return false;
}

void UHTTankMovementComponent::DebugDrawForwardAndVelocity() const
{
	DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + Velocity.GetSafeNormal() * 1000, 10, FColor::Purple, false, 0, 0, 4);
	DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 1000, 10, FColor::Green, false, 0, 0, 4);
}
