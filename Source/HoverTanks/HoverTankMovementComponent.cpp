// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankMovementComponent.h"

#include "HoverTank.h"
#include "Animation/AnimNode_TransitionPoseEvaluator.h"
#include "Components/BoxComponent.h"
#include "GameFramework/GameStateBase.h"

// Sets default values for this component's properties
UHoverTankMovementComponent::UHoverTankMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHoverTankMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	AHoverTank* HoverTank = Cast<AHoverTank>(GetOwner());
	if (HoverTank)
	{
		TankCannonMesh = HoverTank->GetTankCannonMesh();
		TankBarrelMesh = HoverTank->GetTankBarrelMesh();
	}
	
}

// Called every frame
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

void UHoverTankMovementComponent::JumpPressed()
{
	// could handle CD
	bJumpOnNextTick = true;
}

/**
 * Movement Simulation covers Throttle and Steering with drift 
 */
void UHoverTankMovementComponent::SimulateMove(FHoverTankMove Move)
{
	/**
	 * Initial Forces
	 */
	FVector ForceOnObject = GetOwner()->GetActorForwardVector() * Move.Throttle * MaxThrottle;
	FVector AirResistance = CalculateAirResistance();
	FVector RollingResistance = CalculateRollingResistance(Move.bIsEBraking);

	ForceOnObject = ForceOnObject + AirResistance + RollingResistance;

	FVector GroundSurfaceNormal;
	float DistanceFromGround;

	bool bIsGrounded = IsGrounded(GroundSurfaceNormal, DistanceFromGround);
	FVector DownForce = CalculateDownForce(Move, bIsGrounded, DistanceFromGround);
	
	FVector Acceleration = (ForceOnObject / Mass) * Move.DeltaTime;
	Velocity = Velocity + Acceleration + DownForce;

	/**
	 * Turning and Rotation
	 */
	FRotator HorizontalRotation;
	FQuat RotationDelta;
	CalculateTurning(Move, HorizontalRotation, RotationDelta);

	FRotator NewActorRotation = HorizontalRotation;
	
	// FRotator InterpolatedRotationTowardSurfaceNormal = CalculateSurfaceNormalRotation(GroundSurfaceNormal, Move.DeltaTime);
	// NewActorRotation = HorizontalRotation + InterpolatedRotationTowardSurfaceNormal;
	//
	// // GetOwner()->SetActorRotation(HorizontalRotation + InterpolatedRotationTowardNormal);
	// UE_LOG(LogTemp, Warning, TEXT("HorizontalRotation: %s"), *HorizontalRotation.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("InterpolatedRotationTowardNormal: %s"), *InterpolatedRotationTowardSurfaceNormal.ToString());

	GetOwner()->SetActorRotation(NewActorRotation);
	Velocity = RotationDelta.RotateVector(Velocity);

	/**
	 * Move the Actor in 3d space
	 */
	FVector Translation = Velocity * Move.DeltaTime * 100; // * 100 to be in meters per seconds
	FHitResult HitResult;
	GetOwner()->AddActorWorldOffset(Translation, true, &HitResult);

	DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + Velocity.GetSafeNormal() * 1000, 10, FColor::Blue, false, 0, 0, 4);
	DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 1000, 10, FColor::Green, false, 0, 0, 4);
	
	if (HitResult.IsValidBlockingHit())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Hit something"));
		
		// DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 25, 10, FColor::Red, false, 1, 0, 1);
		// DrawDebugDirectionalArrow(GetWorld(), HitResult.ImpactPoint, HitResult.ImpactPoint + HitResult.ImpactNormal * 100, 100, FColor::Green, false, 1, 0, 1);
		FVector BounceVector = CalculateBounceVector(Velocity, HitResult.ImpactNormal);
		// DrawDebugDirectionalArrow(GetWorld(), HitResult.ImpactPoint, HitResult.ImpactPoint + BounceVector * 1000, 200, FColor::Red, false, 1, 0, 2);
		// Velocity = FVector::ZeroVector;
		Velocity = BounceVector * Velocity.Size() / BounceDampening;
	}
}

/**
 * Cannon Rotation Simulation covers the rotation of the TankCannon and TankBarrel meshes
 */
void UHoverTankMovementComponent::SimulateCannonRotate(const FHoverTankCannonRotate& CannonRotate)
{
	FString RoleString;
	UEnum::GetValueAsString(GetOwner()->GetLocalRole(), RoleString);
	// UE_LOG(LogTemp, Warning, TEXT("Role: %s, ControlRotation: %s"), *RoleString,  *CannonRotate.ControlRotation.ToString());
	
	FRotator CannonRotation = TankCannonMesh->GetComponentRotation();
	CannonRotation.Yaw = CannonRotate.ControlRotation.Yaw;
	TankCannonMesh->SetWorldRotation(CannonRotation);

	// try to rotate the TankBarrelMesh toward the ControlRotation
	FRotator BarrelRotation = TankBarrelMesh->GetComponentRotation();
	FRotator BarrelRotationTowardControlRotation = FMath::RInterpTo(BarrelRotation, CannonRotate.ControlRotation, CannonRotate.DeltaTime, 10);
	BarrelRotationTowardControlRotation.Pitch = BarrelRotationTowardControlRotation.Pitch + .25f; // zeroing 
	BarrelRotationTowardControlRotation.Yaw = CannonRotate.ControlRotation.Yaw;
	BarrelRotationTowardControlRotation.Roll = 0;

	// Clamp Pitch between -10 and 15 degrees
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
	Move.bJumpOnNextTick = bJumpOnNextTick;
	Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	bJumpOnNextTick = false;

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

void UHoverTankMovementComponent::CalculateTurning(const FHoverTankMove& Move, FRotator &HorizontalRotation, FQuat &RotationDelta)
{
	// Rotate Actor based on Steering
	HorizontalRotation = GetOwner()->GetActorRotation();
	float RotationAngle = BaseTurnRate * Move.Steering * Move.DeltaTime;
	HorizontalRotation.Yaw += RotationAngle;
	// GetOwner()->SetActorRotation(HorizontalRotation);

	// Rotate Velocity based on Steering, and Drift Ratio
	// As velocity appraoches 0, DriftDivider should approach 1, and as Velocity approaches 20 DriftDivider should approach 2
	float DriftDivider = MaxDriftRatio > KINDA_SMALL_NUMBER
		? 1 + (Velocity.Size() / 20) * MaxDriftRatio
		: 1;
	
	RotationAngle = FMath::DegreesToRadians(RotationAngle) / DriftDivider;
	RotationDelta = FQuat(GetOwner()->GetActorUpVector(), RotationAngle);

	// Velocity = RotationDelta.RotateVector(Velocity);
}

FRotator UHoverTankMovementComponent::CalculateSurfaceNormalRotation(const FVector& GroundSurfaceNormal, float DeltaTime)
{
	FRotator InterpolatedRotationTowardNormal;
	
	// calculate a new actor rotation that pitches toward the surface normal

	// calculate the new rotation that keeps the Actor perpendicular to the surface, and orthogonal to the surface normal
	FQuat RotationDifference = FQuat::FindBetweenNormals(GetOwner()->GetActorUpVector().GetSafeNormal(), GroundSurfaceNormal);
	// FQuat SurfaceNormalRotation = FQuat::Slerp(GetOwner()->GetActorRotation().Quaternion(), RotationDifference, 0.1f);
	FRotator RotationTowardNormal = RotationDifference.Rotator();

	InterpolatedRotationTowardNormal = FMath::RInterpTo(FRotator::ZeroRotator, RotationTowardNormal, DeltaTime, 5);
	
	// interpolate toward this new rotation
	// InterpolatedRotationTowardNormal = FMath::RInterpTo(GetOwner()->GetActorRotation(), SurfaceNormalRotation.Rotation(), GetWorld()->GetDeltaSeconds(), 2);


	return InterpolatedRotationTowardNormal;
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

bool UHoverTankMovementComponent::IsGrounded(FVector &GroundSurfaceNormal, float &DistanceFromGround)
{
	if (GetOwner() == nullptr)
	{
		return false;
	}

	float GroundCheckDistance = 20000.f;
	float GroundNormalThreshold = 0.7f;
	
	// Set up the parameters for the line trace
	FVector StartLocation = GetOwner()->GetActorLocation();
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

FVector UHoverTankMovementComponent::CalculateDownForce(const FHoverTankMove& Move, bool bIsGrounded, float DistanceFromGround)
{
	FVector Gravity = GetWorld()->GetGravityZ() / 100 * FVector(0, 0, 1);

	/**
	 * TODO: create a scene component at the bottom, and trace from there
	 */
	DistanceFromGround = DistanceFromGround - 75;
	
	float DesiredFloatHeight = 100; // 1.0 meters todo make property
	float HoveBounceDivider = 3; // 1 - 6 todo make property

	float CurrentUpDraft = ((1 - (DistanceFromGround / DesiredFloatHeight)) * -Gravity.Z) / HoveBounceDivider;
	FVector DownForce = FVector(0, 0, 1) * CurrentUpDraft * Move.DeltaTime;

	if (Move.bJumpOnNextTick)
	{
		DownForce = DownForce + FVector(0, 0, 1) * 2;
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("DST: %f, CurrentUpDraft: %f, DownForce: %s"), DistanceFromGround, CurrentUpDraft, *DownForce.ToString());
		
	return DownForce;
	

	// if (DistanceFromGround > 800)
	// {
	// 	return Gravity;
	// }
	// else
	// {
	// 	float CurrentUpDraftMultiplier = FMath::Clamp(1 - (DistanceFromGround / 200), 0, 1);
	// 	// UE_LOG(LogTemp, Warning, TEXT("CurrentUpDraftMultiplier: %f"), CurrentUpDraftMultiplier);
	// 	
	//
	// 	FVector UpDraft = FVector(0, 0, 1) * (MaxUpDraftForce + MaxUpDraftForce * CurrentUpDraftMultiplier);
	// 	// FVector UpDraft = (FVector(0, 0, 1) * (MaxUpDraftForce * 100) * 1) / Move.DeltaTime;
	//
	// 	UE_LOG(LogTemp, Warning, TEXT("Distance from ground: %f, CurrentUpDraftMultiplier: %f, UpDraftForce %f"), DistanceFromGround, CurrentUpDraftMultiplier, MaxUpDraftForce);
	// 	UE_LOG(LogTemp, Warning, TEXT("Gravity: %s, UpDraft %s"), *Gravity.ToString(), *UpDraft.ToString());
	//
	// 	return Gravity + UpDraft;
	// }


	// Calculate updraft against gravity. When DistanceFromGround is 0, the updraft should be big enough to counter
	// gravity and lift the tank. When the DistanceFromGround reaches 200 units, the updraft should be the same as gravity
	// in order for the tank to hover.
	
	FVector AntiGravity = -Gravity;
	FVector UpDraft = FVector(0, 0, 1);

	UE_LOG(LogTemp, Warning, TEXT("dst: %f, Gravity: %s, UpDraft %s"), DistanceFromGround, *Gravity.ToString(), *UpDraft.ToString());
	
	return Gravity + UpDraft;
	
	// return bIsGrounded
	// 	? GetOwner()->GetActorUpVector().GetSafeNormal() * MaxUpDraftForce * Move.DeltaTime // float the tank	
	// 	: GetWorld()->GetGravityZ() / 100 * GetOwner()->GetActorUpVector() * Move.DeltaTime; // apply gravity
}

