// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HoverTankMovementComponent.generated.h"

USTRUCT()
struct FHoverTankMove
{
	GENERATED_BODY()

	UPROPERTY()
	float Throttle;
	UPROPERTY()
	float Steering;

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
	float BaseTurnRate = 60;

	/** The mass of the tank (kg). */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Mass = 5000;

	/** Higher values means more drag. kg/m */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPricateAccess = "true"))
	float DragCoefficient = 320;

	float Throttle;
	float Steering;

	/** Cannon turn rate in degrees per second, looking left and right */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CannonTurnRate = 90;

	/** Barrel pitch rate in degrees per second, looking up and down */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float BarrelPitchRate = 90;

	float LookUp;
	float LookRight;
	
	FVector Velocity;

	FVector CalculateBounceVector(const FVector& InVelocity, const FVector& WallNormal);

	FHoverTankMove LastMove;
	FHoverTankMove CreateMove(float DeltaTime);

	UPROPERTY()
	UStaticMeshComponent* TankCannonMesh;

	UPROPERTY()
	UStaticMeshComponent* TankBarrelMesh;

	FHoverTankCannonRotate LastCannonRotate;
	FHoverTankCannonRotate CreateCannonRotate(float DeltaTime);
		
};
