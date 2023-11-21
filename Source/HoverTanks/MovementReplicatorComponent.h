// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HoverTankMovementComponent.h"
#include "Components/ActorComponent.h"
#include "MovementReplicatorComponent.generated.h"

USTRUCT()
struct FHoverTankMoveState
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FHoverTankMove LastMove;
};

/**
 * Based on the KrazyKarts Course
 */
struct FHermiteCubicSpline
{
	FVector StartLocation, StartDerivative, TargetLocation, TargetDerivative;

	FVector InterpolateLocation(float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}

	FVector InterpolateDerivative(float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOVERTANKS_API UMovementReplicatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMovementReplicatorComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UHoverTankMovementComponent* HoverTankMovementComponent;

	void ClientTick(float DeltaTime);
	
	UPROPERTY(ReplicatedUsing=OnRep_ServerMoveState)
	FHoverTankMoveState ServerMoveState;
	
	UFUNCTION() // must be a UFUNCTION
	void OnRep_ServerMoveState();
	void SimulatedProxy_OnRep_ServerMoveState();
	void AutonomousProxy_OnRep_ServerMoveState();
	
	TArray<FHoverTankMove> UnacknowledgedMoves;
	
	void ClearAcknowledgedMoves(FHoverTankMove LastMove);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSendMove(FHoverTankMove Move);

	void UpdateServerMoveState(const FHoverTankMove& Move);
	
	/**
	 * Client Interpolation
	 */
	float ClientTimeSinceUpdate;
	float ClientTimeBetweenLastUpdates;
	FTransform ClientStartTransform;
	FVector ClientStartVelocity;

	FHermiteCubicSpline CreateSpline();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSendCannonRotate(FHoverTankCannonRotate CannonRotate);

	UPROPERTY(ReplicatedUsing=OnRep_LastCannonRotate)
	FHoverTankCannonRotate LastCannonRotate;
	
	UFUNCTION()
	void OnRep_LastCannonRotate();

		
};
