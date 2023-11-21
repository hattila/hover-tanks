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
		// HoverTankMovementComponent->SimulateMove(ServerState.LastMove);
		// DrawDebugString(GetWorld(), FVector(0, 0, 200), TEXT("This one shall be smooth"), this, FColor::White, 0);

		ClientTick(DeltaTime);
	}
	
}

void UMovementReplicatorComponent::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER)
	{
		return;
	}

	if (HoverTankMovementComponent == nullptr)
	{
		return;
	}

	float LerpRatio = FMath::Clamp(ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates, 0.f, 1.f);

	FHermiteCubicSpline Spline = CreateSpline();

	// interpolate Location
	FVector InterpolatedLocation = Spline.InterpolateLocation(LerpRatio);
	GetOwner()->SetActorLocation(InterpolatedLocation);
	
	// interpolate Velocity
	FVector InterpolatedDerivative = Spline.InterpolateDerivative(LerpRatio);
	FVector InterpolatedVelocity = InterpolatedDerivative / (ClientTimeBetweenLastUpdates * 100); // m to cm
	HoverTankMovementComponent->SetVelocity(InterpolatedVelocity);
	
	// interpolate Rotation
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat StartRotation = ClientStartTransform.GetRotation();
	// spherical linear interpolation
	FQuat InterpolatedRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);

	GetOwner()->SetActorRotation(InterpolatedRotation);
}

void UMovementReplicatorComponent::OnRep_ServerState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	default:
		break;
	}
}

/**
 * Simulated Proxies cache the last server state and use it to interpolate to the current state on ClientTick.
 */
void UMovementReplicatorComponent::SimulatedProxy_OnRep_ServerState()
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
void UMovementReplicatorComponent::AutonomousProxy_OnRep_ServerState()
{
	if (HoverTankMovementComponent == nullptr)
	{
		return;
	}

	GetOwner()->SetActorTransform(ServerState.Transform);
	HoverTankMovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowledgedMoves(ServerState.LastMove);

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

FHermiteCubicSpline UMovementReplicatorComponent::CreateSpline()
{
	FHermiteCubicSpline Spline;

	Spline.StartLocation = ClientStartTransform.GetLocation();
	Spline.TargetLocation = ServerState.Transform.GetLocation();

	Spline.TargetDerivative = ServerState.Velocity * ClientTimeBetweenLastUpdates * 100; // cm to m
	Spline.StartDerivative = ClientStartVelocity * ClientTimeBetweenLastUpdates * 100; // cm to m
	
	return Spline;
}