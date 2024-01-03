// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankEffectsComponent.h"

#include "EngineUtils.h"
#include "HoverTankMovementComponent.h"
#include "MovementReplicatorComponent.h"
#include "HoverTanks/Game/InTeamPlayerState.h"
#include "HoverTanks/Game/GameModes/TeamDeathMatchGameState.h"
#include "HoverTanks/Game/Teams/Team.h"
#include "HoverTanks/Game/Teams/TeamDataAsset.h"
#include "Net/UnrealNetwork.h"


UHoverTankEffectsComponent::UHoverTankEffectsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// void UHoverTankEffectsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
// {
// 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
// 	DOREPLIFETIME(UHoverTankEffectsComponent, TankLightsDynamicMaterialInstance);
// }

void UHoverTankEffectsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementReplicatorComponent)
	{
		FHoverTankMove LastMove = MovementReplicatorComponent->GetHoverTankMoveState().LastMove;

		if (LastMove.bIsEBraking)
		{
			BrakeLights(true);
		}
		else
		{
			BrakeLights(false);
		}

		if (LastMove.bIsBoosting || LastMove.bIsJumping)
		{
			ThrusterLights(true);
		}
		else
		{
			ThrusterLights(false);
		}
	}
}

void UHoverTankEffectsComponent::BrakeLights(const bool bBraking) const
{

	if (TankLightsDynamicMaterialInstance)
	{
		const float Strength = bBraking ? TankLightsBrakeMaxStrength : TankLightsBrakeDefaultStrength;
		TankLightsDynamicMaterialInstance->SetScalarParameterValue(TankLightsBrakeStrengthName, Strength);
	}
}

void UHoverTankEffectsComponent::ThrusterLights(const bool bThrusting) const
{
	if (TankLightsDynamicMaterialInstance)
	{
		const float Strength = bThrusting ? TankLightsThrusterMaxStrength : TankLightsThrusterDefaultStrength;
		TankLightsDynamicMaterialInstance->SetScalarParameterValue(TankLightsThrusterStrengthName, Strength);
	}
}

void UHoverTankEffectsComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementReplicatorComponent = GetOwner()->FindComponentByClass<UMovementReplicatorComponent>();

	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetOwner()->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
	
	TankLightsDynamicMaterialInstance = StaticMeshComponents[1]->CreateDynamicMaterialInstance(1);
	
	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
	{
		StaticMeshComponent->SetMaterial(1, TankLightsDynamicMaterialInstance);
	}

	//
	// if (GetOwnerRole() == ROLE_AutonomousProxy)
	// {
	// }
	

	
	// FString RoleString;
	// UEnum::GetValueAsString(GetOwner()->GetLocalRole(), RoleString);
	//
	//
	// APawn* Owner = Cast<APawn>(GetOwner());
	// if (Owner == nullptr)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("FX comp, %s Owner is not a Pawn"), *RoleString);
	// 	return;
	// }
	//
	// AController* OwnerController = Owner->GetController();
	// APlayerController* PlayerController = Cast<APlayerController>(OwnerController);
	//
	// if (PlayerController == nullptr)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("FX comp, %s Owner Has no player controller"), *RoleString);
	// 	return;
	// }
	//
	// // get the player state
	// AInTeamPlayerState* TeamPlayerState = PlayerController->GetPlayerState<AInTeamPlayerState>();
	// if (TeamPlayerState)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("FX comp %s, team id: %d, subscribing to onTeamChange"), *RoleString, TeamPlayerState->GetTeamId());
	// 	TeamPlayerState->OnTeamIdChanged.AddDynamic(this, &UHoverTankEffectsComponent::OnTeamIdChanged);
	//
	// 	OnTeamIdChanged(TeamPlayerState->GetTeamId());
	// }

	
}

void UHoverTankEffectsComponent::OnTeamIdChanged(int8 NewTeamId)
{
	// get Owner actor name and role
	FString RoleString;
	UEnum::GetValueAsString(GetOwner()->GetLocalRole(), RoleString);
	UE_LOG(LogTemp, Warning, TEXT("FX comp, OnTeamIdChanged Owner: %s, Role: %s, new TeamId: %d"), *GetOwner()->GetName(), *RoleString, NewTeamId);
	
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("FX comp, OnTeamIdChanged no world"));
		return;
	}

	// get hold of the game state
	ATeamDeathMatchGameState* GameState = GetWorld()->GetGameState<ATeamDeathMatchGameState>();
	TMap<int8, ATeam*> TeamMap = GameState->GetTeamMap();

	ATeam** Team = TeamMap.Find(NewTeamId);
	UTeamDataAsset* TeamDataAsset = Team ? (*Team)->GetTeamDataAsset() : nullptr;
	if (TeamDataAsset)
	{
		ApplyTeamColors(TeamDataAsset);
	}
}

void UHoverTankEffectsComponent::ApplyTeamColors(UTeamDataAsset* TeamDataAsset)
{
	if (!TeamDataAsset)
	{
		FString RoleString;
		UEnum::GetValueAsString(GetOwner()->GetLocalRole(), RoleString);
		UE_LOG(LogTemp, Warning, TEXT("FX comp, ApplyTeamColors, no team data asset, role %s"), *RoleString);
		return;
	}
	
	TeamDataAsset->ApplyToActor(GetOwner());
	MulticastApplyTeamColors(TeamDataAsset);
}

void UHoverTankEffectsComponent::MulticastApplyTeamColors_Implementation(UTeamDataAsset* TeamDataAsset)
{
	UE_LOG(LogTemp, Warning, TEXT("FX comp, MulticastApplyTeamColors_Implementation, color %s"), *TeamDataAsset->GetTeamShortName().ToString());
	TeamDataAsset->ApplyToActor(GetOwner());
}
