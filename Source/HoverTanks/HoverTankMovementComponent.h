// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HoverTankMovementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOVERTANKS_API UHoverTankMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHoverTankMovementComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// define setter for Throttle
	void SetThrottle(float InThrottle) { Throttle = InThrottle; }
	// define setter for Steering
	void SetSteering(float InSteering) { Steering = InSteering; }
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	/** In Newtons. The max driving force */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MaxThrottle = 40000;

	/** Tank turn rate in degrees per second */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate = 45;

	/** The mass of the tank (kg). */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Mass = 5000;

	/** Higher values means more drag. kg/m */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPricateAccess = "true"))
	float DragCoefficient = 320;

	float Throttle;
	float Steering;
	
	FVector Velocity;

	FVector CalculateBounceVector(const FVector& InVelocity, const FVector& WallNormal);
	

		
};
