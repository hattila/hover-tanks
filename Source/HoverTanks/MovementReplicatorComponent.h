// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HoverTankMovementComponent.h"
#include "Components/ActorComponent.h"
#include "MovementReplicatorComponent.generated.h"

USTRUCT()
struct FHoverTankState
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FHoverTankMove LastMove;
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

	UPROPERTY(ReplicatedUsing=OnRep_ServerState)
	FHoverTankState ServerState;

	UFUNCTION() // must be a UFUNCTION
	void OnRep_ServerState();
	void SimulatedProxy_OnRep_ServerState();
	void AutonomousProxy_OnRep_ServerState();
	
	TArray<FHoverTankMove> UnacknowledgedMoves;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSendMove(FHoverTankMove Move);

	void UpdateServerState(const FHoverTankMove& Move);

		
};
