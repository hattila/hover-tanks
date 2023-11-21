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

void UHoverTankMovementComponent::SimulateMove(FHoverTankMove Move)
{
	/**
	 * FORWARD MOVEMENT AND TURNING
	 */
	FVector ForceOnObject = GetOwner()->GetActorForwardVector() * Move.Throttle * MaxThrottle;
	FVector AirResistance = Velocity.GetSafeNormal() * -1 * Velocity.SizeSquared() * DragCoefficient;

	ForceOnObject = ForceOnObject + AirResistance;

	FVector Acceleration = ForceOnObject / Mass;
	Velocity = Velocity + Acceleration * Move.DeltaTime;

	// Rotate Actor based on Steering
	FRotator Rotation = GetOwner()->GetActorRotation();
	float YawRotation = Move.Throttle >= 0
		                    ? Move.Steering * BaseTurnRate * Move.DeltaTime
		                    : -1 * Move.Steering * BaseTurnRate * Move.DeltaTime; // 90 degrees per second
	
	Rotation.Yaw += YawRotation;
	GetOwner()->SetActorRotation(Rotation);
	
	// Move the Actor
	FVector Translation = Velocity * Move.DeltaTime * 100; // * 100 to be in meters per seconds
	FHitResult HitResult;
	GetOwner()->AddActorWorldOffset(Translation, true, &HitResult);

	// DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + Velocity.GetSafeNormal() * 1000, 10, FColor::Blue, false, 0, 0, 4);
	
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
 * ROTATE CANNON AND BARREL WITH CAMERA
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

FVector UHoverTankMovementComponent::CalculateBounceVector(const FVector& InVelocity, const FVector& WallNormal)
{
	// Ensure that the incoming velocity and wall normal are normalized
	FVector NormalizedVelocity = InVelocity.GetSafeNormal();
	FVector NormalizedWallNormal = WallNormal.GetSafeNormal();

	// Calculate the reflection vector using the formula: R = I - 2 * (I dot N) * N
	FVector BounceVector = NormalizedVelocity - 2.0f * FVector::DotProduct(NormalizedVelocity, NormalizedWallNormal) * NormalizedWallNormal;

	return BounceVector;
}

FHoverTankMove UHoverTankMovementComponent::CreateMove(float DeltaTime)
{
	FHoverTankMove Move;
	Move.DeltaTime = DeltaTime;
	Move.Throttle = Throttle;
	Move.Steering = Steering;
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

