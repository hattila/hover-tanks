// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankMovementComponent.h"

#include "HoverTank.h"
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

		LastCannonRotate = CreateCannonRotate(DeltaTime);
		SimulateCannonRotate(LastCannonRotate);
	}
}

/**
 * Movement Simulation covers Throttle and Steering with drift 
 */
void UHoverTankMovementComponent::SimulateMove(FHoverTankMove Move)
{
	FVector ForceOnObject = GetOwner()->GetActorForwardVector() * Move.Throttle * MaxThrottle;

	FVector AirResistance = CalculateAirResistance();
	FVector RollingResistance = CalculateRollingResistance();

	ForceOnObject = ForceOnObject + AirResistance + RollingResistance;

	FVector Acceleration = (ForceOnObject / Mass) * Move.DeltaTime;
	FVector DownForce = CalculateDownForce(Move);

	Velocity = Velocity + Acceleration + DownForce;

	SimulateTurning(Move);

	// Move the Actor
	FVector Translation = Velocity * Move.DeltaTime * 100; // * 100 to be in meters per seconds
	FHitResult HitResult;
	GetOwner()->AddActorWorldOffset(Translation, true, &HitResult);

	DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + Velocity.GetSafeNormal() * 1000, 10, FColor::Blue, false, 0, 0, 4);
	
	if (HitResult.IsValidBlockingHit())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Hit something"));
		
		// DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 25, 10, FColor::Red, false, 1, 0, 1);
		// DrawDebugDirectionalArrow(GetWorld(), HitResult.ImpactPoint, HitResult.ImpactPoint + HitResult.ImpactNormal * 100, 100, FColor::Green, false, 1, 0, 1);
		FVector BounceVector = CalculateBounceVector(Velocity, HitResult.ImpactNormal);
		// DrawDebugDirectionalArrow(GetWorld(), HitResult.ImpactPoint, HitResult.ImpactPoint + BounceVector * 1000, 200, FColor::Red, false, 1, 0, 2);
		// Velocity = FVector::ZeroVector;
		Velocity = BounceVector * Velocity.Size() / 2;
	}
}

/**
 * Cannon Rotation Simulation covers the rotation of the TankCannon and TankBarrel meshes
 */
void UHoverTankMovementComponent::SimulateCannonRotate(const FHoverTankCannonRotate& CannonRotate)
{
	// Rotate the TankCannon mesh based on the LookRight input
	FRotator CannonRotation = TankCannonMesh->GetComponentRotation();
	float CannonYawRotation = CannonRotate.LookRight * CannonTurnRate * CannonRotate.DeltaTime; // 90 degrees per second
	CannonRotation.Yaw += CannonYawRotation;
	TankCannonMesh->SetWorldRotation(CannonRotation);

	// Rotate the TankBarrel mesh up and down based on LookUp input, with a maximum of 15 degrees up and -10 degrees down
	FRotator BarrelRotation = TankBarrelMesh->GetComponentRotation();
	float BarrelPitchRotation = CannonRotate.LookUp * BarrelPitchRate * CannonRotate.DeltaTime; // 90 degrees per second
	BarrelRotation.Pitch += BarrelPitchRotation;
	BarrelRotation.Pitch = FMath::Clamp(BarrelRotation.Pitch, -10.0f, 15.0f);
	TankBarrelMesh->SetWorldRotation(BarrelRotation);
}

FHoverTankMove UHoverTankMovementComponent::CreateMove(float DeltaTime)
{
	FHoverTankMove Move;
	Move.DeltaTime = DeltaTime;
	Move.Throttle = Throttle;
	Move.Steering = Steering;
	Move.bIsEBraking = bIsEBraking;
	Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	
	return Move;
}

FHoverTankCannonRotate UHoverTankMovementComponent::CreateCannonRotate(float DeltaTime)
{
	FHoverTankCannonRotate CannonRotate;
	CannonRotate.DeltaTime = DeltaTime;
	CannonRotate.LookUp = LookUp;
	CannonRotate.LookRight = LookRight;

	return CannonRotate;
}

void UHoverTankMovementComponent::SimulateTurning(const FHoverTankMove& Move)
{
	// Rotate Actor based on Steering
	FRotator Rotation = GetOwner()->GetActorRotation();
	float RotationAngle = BaseTurnRate * Move.Steering * Move.DeltaTime;
	Rotation.Yaw += RotationAngle;
	GetOwner()->SetActorRotation(Rotation);

	// Rotate Velocity based on Steering, and Drift Ratio
	
	// As velocity appraoches 0, DriftDivider should approach 1, and as Velocity approaches 20 DriftDivider should approach 2
	float DriftDivider = MaxDriftRatio > KINDA_SMALL_NUMBER
		? 1 + (Velocity.Size() / 20) * MaxDriftRatio
		: 1;
	
	RotationAngle = FMath::DegreesToRadians(RotationAngle) / DriftDivider;
	FQuat RotationDelta(GetOwner()->GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);
}

FVector UHoverTankMovementComponent::CalculateAirResistance()
{
	return Velocity.GetSafeNormal() * -1 * Velocity.SizeSquared() * DragCoefficient;
}

FVector UHoverTankMovementComponent::CalculateRollingResistance()
{
	float AccelerationDueToGravity = GetWorld()->GetGravityZ() / 100; // 1 to 100 to be in meters per seconds
	
	float NormalForce = Mass * AccelerationDueToGravity;
	
	if (bIsEBraking)
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

bool UHoverTankMovementComponent::IsGrounded()
{
	// Get the pawn owner
	// APawn* OwnerPawn = Cast<APawn>(GetOwner());
	// if (!OwnerPawn)
	// {
	// 	// Handle the case where there is no valid pawn owner
	// 	return false;
	// }

	

	float GroundCheckDistance = 200.f;
	float GroundNormalThreshold = 0.7f;
	
	// Set up the parameters for the line trace
	FVector StartLocation = GetOwner()->GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0, 0, GroundCheckDistance); // Adjust the Z component based on your needs

	// Perform a line trace to check for the ground
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());

	// Uncomment the next line for debugging purposes
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green, false, 1, 0, 1);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams))
	{
		// Check if the hit surface is walkable (you may need to adjust this based on your game)
		if (HitResult.ImpactNormal.Z >= GroundNormalThreshold)
		{
			// The pawn is considered grounded
			return true;
		}
	}

	// The pawn is not grounded
	return false;
}

FVector UHoverTankMovementComponent::CalculateDownForce(const FHoverTankMove& Move)
{
	return IsGrounded()
		? GetOwner()->GetActorUpVector().GetSafeNormal() * UpDraft * Move.DeltaTime // float the tank	
		: GetWorld()->GetGravityZ() / 100 * GetOwner()->GetActorUpVector() * Move.DeltaTime; // apply gravity
}
