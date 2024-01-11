// Fill out your copyright notice in the Description page of Project Settings.


#include "MovementReplicatorComponent.h"

#include "Net/UnrealNetwork.h"

UMovementReplicatorComponent::UMovementReplicatorComponent(): ServerMoveState(),
                                                              ClientTimeSinceUpdate(0),
                                                              ClientTimeBetweenLastUpdates(0),
                                                              ServerCannonRotateState()
{
	PrimaryComponentTick.bCanEverTick = true;
	// SetIsReplicated(true); // it is set in the parent actors constructor	
}

void UMovementReplicatorComponent::BeginPlay()
{
	Super::BeginPlay();

	HoverTankMovementComponent = GetOwner()->FindComponentByClass<UHoverTankMovementComponent>();
}

void UMovementReplicatorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMovementReplicatorComponent, ServerMoveState);
	DOREPLIFETIME(UMovementReplicatorComponent, ServerCannonRotateState);
}

void UMovementReplicatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (HoverTankMovementComponent == nullptr)
	{
		return;
	}

	FHoverTankMove LastMove = HoverTankMovementComponent->GetLastMove();
	FHoverTankCannonRotate LastCannonRotate = HoverTankMovementComponent->GetLastCannonRotate();

	// FString RoleString;
	// UEnum::GetValueAsString(GetOwner()->GetLocalRole(), RoleString);
	// UE_LOG(LogTemp, Warning, TEXT("Replicator gets LastCannonRotate as %s, on tick. LookUp: %f"), *RoleString, LastCannonRotate.LookUp);

	/**
	 * Owning Client
	 */
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(LastMove);
		ServerSendMove(LastMove);
		
		ServerSendCannonRotate(LastCannonRotate);
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
		UpdateServerMoveState(LastMove);
		UpdateServerCannonRotate(LastCannonRotate);
	}

	/**
	 * Other Clients seen by a Client
	 */
	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}
	
}

void UMovementReplicatorComponent::ClientTick(const float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER)
	{
		return;
	}

	float LerpRatio = FMath::Clamp(ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates, 0.f, 1.f);
	
	InterpolateMovement(LerpRatio);
	InterpolateCannon(LerpRatio);
}

void UMovementReplicatorComponent::InterpolateMovement(const float LerpRatio)
{
	if (HoverTankMovementComponent == nullptr)
	{
		return;
	}

	const FHermiteCubicSpline Spline = CreateSpline();

	// interpolate Location
	FVector InterpolatedLocation = Spline.InterpolateLocation(LerpRatio);
	GetOwner()->SetActorLocation(InterpolatedLocation);
	
	// interpolate Velocity
	FVector InterpolatedDerivative = Spline.InterpolateDerivative(LerpRatio);
	FVector InterpolatedVelocity = InterpolatedDerivative / (ClientTimeBetweenLastUpdates * 100); // m to cm
	HoverTankMovementComponent->SetVelocity(InterpolatedVelocity);
	
	// interpolate Rotation
	FQuat TargetRotation = ServerMoveState.Transform.GetRotation();
	FQuat StartRotation = ClientStartTransform.GetRotation();
	// spherical linear interpolation
	FQuat InterpolatedRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);

	GetOwner()->SetActorRotation(InterpolatedRotation);
}

void UMovementReplicatorComponent::InterpolateCannon(const float LerpRatio)
{
	if (HoverTankMovementComponent == nullptr || HoverTankMovementComponent->GetTankCannonMesh() == nullptr || HoverTankMovementComponent->GetTankBarrelMesh() == nullptr)
	{
		return;
	}

	// FRotator InterpolatedCannonRotation = FMath::Lerp(ClientStartCannonRotation, ServerCannonRotateState.CannonRotation, LerpRatio);
	FQuat InterpolatedCannonRotation = FQuat::Slerp(ClientStartCannonRotation.Quaternion(), ServerCannonRotateState.CannonRotation.Quaternion(), LerpRatio);
	HoverTankMovementComponent->GetTankCannonMesh()->SetWorldRotation(InterpolatedCannonRotation);

	FRotator InterpolatedBarrelRotation = FMath::Lerp(ClientStartBarrelRotation, ServerCannonRotateState.BarrelRotation, LerpRatio);
	HoverTankMovementComponent->GetTankBarrelMesh()->SetWorldRotation(InterpolatedBarrelRotation);
}

void UMovementReplicatorComponent::OnRep_ServerMoveState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerMoveState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerMoveState();
		break;
	default:
		break;
	}
}

/**
 * Simulated Proxies cache the last server state and use it to interpolate to the current state on ClientTick.
 */
void UMovementReplicatorComponent::SimulatedProxy_OnRep_ServerMoveState()
{
	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate; // Time between last update
	ClientTimeSinceUpdate = 0; // we just received an update

	ClientStartTransform = GetOwner()->GetTransform();

	if (HoverTankMovementComponent)
	{
		ClientStartVelocity = HoverTankMovementComponent->GetVelocity();
	}
}

/**
 * Autonomous Proxies get set back where the serves state says we are, and than plays the saved unacknowledged
 * moves on top.
 */
void UMovementReplicatorComponent::AutonomousProxy_OnRep_ServerMoveState()
{
	if (HoverTankMovementComponent == nullptr)
	{
		return;
	}

	GetOwner()->SetActorTransform(ServerMoveState.Transform);
	HoverTankMovementComponent->SetVelocity(ServerMoveState.Velocity);

	ClearAcknowledgedMoves(ServerMoveState.LastMove);

	/**
	 * The higher the latency (lag) the more moves we have to simulate.
	 */
	for (const FHoverTankMove& Move : UnacknowledgedMoves)
	{
		HoverTankMovementComponent->SimulateMove(Move);
	}
}

void UMovementReplicatorComponent::ClearAcknowledgedMoves(FHoverTankMove LastMove)
{
	// iterate over UnacknowledgedMoves and remove every element with older time than in LastMove
	TArray<FHoverTankMove> NewMoves;
	for (const FHoverTankMove& Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}
	UnacknowledgedMoves = NewMoves;
}

void UMovementReplicatorComponent::ServerSendMove_Implementation(FHoverTankMove Move)
{
	if (HoverTankMovementComponent == nullptr)
	{
		return;
	}

	HoverTankMovementComponent->SimulateMove(Move);
	UpdateServerMoveState(Move);
}

bool UMovementReplicatorComponent::ServerSendMove_Validate(FHoverTankMove Move)
{
	return Move.IsValid();
}

void UMovementReplicatorComponent::UpdateServerMoveState(const FHoverTankMove& Move)
{
	ServerMoveState.LastMove = Move;
	ServerMoveState.Transform = GetOwner()->GetActorTransform();
	ServerMoveState.Velocity = HoverTankMovementComponent->GetVelocity();
}

FHermiteCubicSpline UMovementReplicatorComponent::CreateSpline()
{
	FHermiteCubicSpline Spline;

	Spline.StartLocation = ClientStartTransform.GetLocation();
	Spline.TargetLocation = ServerMoveState.Transform.GetLocation();

	Spline.TargetDerivative = ServerMoveState.Velocity * ClientTimeBetweenLastUpdates * 100; // cm to m
	Spline.StartDerivative = ClientStartVelocity * ClientTimeBetweenLastUpdates * 100; // cm to m
	
	return Spline;
}

void UMovementReplicatorComponent::ServerSendCannonRotate_Implementation(const FHoverTankCannonRotate& CannonRotate)
{
	if (HoverTankMovementComponent == nullptr)
	{
		return;
	}

	HoverTankMovementComponent->SimulateCannonRotate(CannonRotate);
	UpdateServerCannonRotate(CannonRotate);
}

bool UMovementReplicatorComponent::ServerSendCannonRotate_Validate(const FHoverTankCannonRotate& CannonRotate)
{
	// return CannonRotate.IsValid();
	return true;
}

void UMovementReplicatorComponent::UpdateServerCannonRotate(FHoverTankCannonRotate LastCannonRotateReceived)
{
	ServerCannonRotateState.LastCannonRotate = LastCannonRotateReceived;
	ServerCannonRotateState.CannonRotation = HoverTankMovementComponent->GetTankCannonMesh()->GetComponentRotation(); // maybe a getter for the rotation only?
	ServerCannonRotateState.BarrelRotation = HoverTankMovementComponent->GetTankBarrelMesh()->GetComponentRotation();
}

void UMovementReplicatorComponent::OnRep_ServerCannonRotateState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerCannonRotateState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerCannonRotateState();
		break;
	default:
		break;
	}
}

void UMovementReplicatorComponent::AutonomousProxy_OnRep_ServerCannonRotateState()
{
	if (HoverTankMovementComponent == nullptr)
	{
		return;
	}

	/**
	 * Does not need to be corrected by the server. Directly controlled by mouse input, so it is always correct.
	 */
	
	// HoverTankMovementComponent->SimulateCannonRotate(ServerCannonRotateState.LastCannonRotate);
}

void UMovementReplicatorComponent::SimulatedProxy_OnRep_ServerCannonRotateState()
{
	if (HoverTankMovementComponent == nullptr || HoverTankMovementComponent->GetTankCannonMesh() == nullptr)
	{
		return;
	}

	ClientStartCannonRotation = HoverTankMovementComponent->GetTankCannonMesh()->GetComponentRotation();
	ClientStartBarrelRotation = HoverTankMovementComponent->GetTankBarrelMesh()->GetComponentRotation();

	// UE_LOG(LogTemp, Warning, TEXT("Simulated Proxies current cannon rotation %s"), *ClientStartCannonRotation.ToString());
}

