// Fill out your copyright notice in the Description page of Project Settings.


#include "MovementReplicatorComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UMovementReplicatorComponent::UMovementReplicatorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);	
}


// Called when the game starts
void UMovementReplicatorComponent::BeginPlay()
{
	Super::BeginPlay();

	HoverTankMovementComponent = GetOwner()->FindComponentByClass<UHoverTankMovementComponent>();
}

void UMovementReplicatorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMovementReplicatorComponent, ServerState);
}

// Called every frame
void UMovementReplicatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (HoverTankMovementComponent == nullptr)
	{
		return;
	}

	FHoverTankMove LastMove = HoverTankMovementComponent->GetLastMove();

	/**
	 * Owning Client
	 */
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(LastMove);
		ServerSendMove(LastMove);
	}

	auto Owner = Cast<APawn>(GetOwner());
	if (Owner == nullptr)
	{
		return;
	}

	/**
	 * The Server controlling the Pawn
	 * IsLocallyControlled is only available on a pawn, and GetOwner gives us the Owner AActor only.
	 */
	// if (GetOwnerRole() == ROLE_Authority && GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	if (GetOwnerRole() == ROLE_Authority && Owner->IsLocallyControlled())
	{
		UpdateServerState(LastMove);
	}

	/**
	 * Other Clients seen by a Client
	 */
	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		HoverTankMovementComponent->SimulateMove(ServerState.LastMove);
		// DrawDebugString(GetWorld(), FVector(0, 0, 200), TEXT("This one shall be smooth"), this, FColor::White, 0);

		// ClientTick(DeltaTime);
	}
	
}

void UMovementReplicatorComponent::OnRep_ServerState()
{
}

void UMovementReplicatorComponent::SimulatedProxy_OnRep_ServerState()
{
}

void UMovementReplicatorComponent::AutonomousProxy_OnRep_ServerState()
{
}

void UMovementReplicatorComponent::ServerSendMove_Implementation(FHoverTankMove Move)
{
	if (HoverTankMovementComponent == nullptr)
	{
		return;
	}

	HoverTankMovementComponent->SimulateMove(Move);
	UpdateServerState(Move);
}

bool UMovementReplicatorComponent::ServerSendMove_Validate(FHoverTankMove Move)
{
	return Move.IsValid();
}

void UMovementReplicatorComponent::UpdateServerState(const FHoverTankMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = HoverTankMovementComponent->GetVelocity();
}