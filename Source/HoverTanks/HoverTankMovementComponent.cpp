// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankMovementComponent.h"

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

	// ...
	
}


// Called every frame
void UHoverTankMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/**
	 * FORWARD MOVEMENT AND TURNING
	 */
	FVector ForceOnObject = GetOwner()->GetActorForwardVector() * Throttle * MaxThrottle;
	FVector AirResistance = Velocity.GetSafeNormal() * -1 * Velocity.SizeSquared() * DragCoefficient;

	ForceOnObject = ForceOnObject + AirResistance;

	FVector Acceleration = ForceOnObject / Mass;
	Velocity = Velocity + Acceleration * DeltaTime;

	// Rotate Actor based on Steering
	FRotator Rotation = GetOwner()->GetActorRotation();
	float YawRotation = Throttle >= 0
		                    ? Steering * BaseTurnRate * DeltaTime
		                    : -1 * Steering * BaseTurnRate * DeltaTime; // 90 degrees per second
	
	Rotation.Yaw += YawRotation;
	GetOwner()->SetActorRotation(Rotation);
	
	// Move the Actor
	FVector Translation = Velocity * DeltaTime * 100; // * 100 to be in meters per seconds
	FHitResult HitResult;
	GetOwner()->AddActorWorldOffset(Translation, true, &HitResult);

	DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + Velocity.GetSafeNormal() * 1000, 10, FColor::Blue, false, 0, 0, 4);
	
	if (HitResult.IsValidBlockingHit())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit something"));
		
		// impact point
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 25, 10, FColor::Red, false, 1, 0, 1);
		// impact normal
		DrawDebugDirectionalArrow(GetWorld(), HitResult.ImpactPoint, HitResult.ImpactPoint + HitResult.ImpactNormal * 100, 100, FColor::Green, false, 1, 0, 1);
		FVector BounceVector = CalculateBounceVector(Velocity, HitResult.ImpactNormal);
		// reflection vector
		DrawDebugDirectionalArrow(GetWorld(), HitResult.ImpactPoint, HitResult.ImpactPoint + BounceVector * 1000, 200, FColor::Red, false, 1, 0, 2);
		// Velocity = FVector::ZeroVector;
		Velocity = BounceVector * Velocity.Size() / 2;
	}
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

