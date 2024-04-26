// Fill out your copyright notice in the Description page of Project Settings.


#include "HTTankEffectsComponent.h"

#include "HTTankMovementComponent.h"
#include "HTMovementReplicatorComponent.h"
#include "HoverTanks/Pawns/HoverTank/HTHoverTank.h"
#include "HoverTanks/Game/Teams/HTTeamDataAsset.h"

#include "NiagaraComponent.h"
#include "Components/RectLightComponent.h"
#include "Net/UnrealNetwork.h"

UHTTankEffectsComponent::UHTTankEffectsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	TankBurningFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Tank Burning FX"));
	TankBurningFX->SetAutoActivate(false);
	TankBurningFX->SetRelativeLocation(TankBurningFXOffset);
	TankBurningFX->SetIsReplicated(true);

	// initialize TankHoverSmokeFX
	TankHoverSmokeFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Tank Hover Smoke FX"));
	TankHoverSmokeFX->SetAutoActivate(true);
	TankHoverSmokeFX->SetIsReplicated(true);
}

void UHTTankEffectsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHTTankEffectsComponent, TeamDataAsset);
	DOREPLIFETIME(UHTTankEffectsComponent, bAreLightsOn);
	DOREPLIFETIME(UHTTankEffectsComponent, TeamColorEmissiveStrength);
	// DOREPLIFETIME(UHTTankEffectsComponent, bIsBurningFxActive);
}


void UHTTankEffectsComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementReplicatorComponent = GetOwner()->FindComponentByClass<UHTMovementReplicatorComponent>();
	TankMovementComponent = MovementReplicatorComponent->GetHoverTankMovementComponent();

	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetOwner()->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
	
	TankLightsDynamicMaterialInstance = StaticMeshComponents[1]->CreateDynamicMaterialInstance(1);
	
	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
	{
		StaticMeshComponent->SetMaterial(1, TankLightsDynamicMaterialInstance);
	}

	AHTHoverTank* HoverTank = Cast<AHTHoverTank>(GetOwner());
	if (HoverTank)
	{
		if (TankBurningFX)
		{
			TankBurningFX->AttachToComponent(HoverTank->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		}

		if (TankHoverSmokeFX)
		{
			TankHoverSmokeFX->AttachToComponent(HoverTank->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			TankHoverSmokeFX->Activate();
		}
	}

	// FString RoleString;
	// UEnum::GetValueAsString(GetOwner()->GetLocalRole(), RoleString);
	// UE_LOG(LogTemp, Warning, TEXT("FX comp, BeginPlay, role %s, TeamColorEmissiveStrength %f"), *RoleString, TeamColorEmissiveStrength);
}

void UHTTankEffectsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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

		if (TankHoverSmokeFX && TankMovementComponent)
		{
			FVector GroundSurfaceNormal;
			float DistanceFromGround;
			bool bIsGrounded = TankMovementComponent->IsGrounded(GroundSurfaceNormal, DistanceFromGround);

			if (!bIsGrounded)
			{
				return;
			}

			float MaxGroundDistance = 400;
			
			float MaxSpawnRate = 20000;
			float SpawnRateMultiplier = FMath::Clamp(1 - DistanceFromGround / MaxGroundDistance, 0, 1);
			float SpawnRateValue = MaxSpawnRate * SpawnRateMultiplier;
			TankHoverSmokeFX->SetFloatParameter(TEXT("SpawnRate"), SpawnRateValue);
			
			float MaxAttractionStrength = 500;
			float MinAttractionStrength = 10;
			float StrengthMultiplier = FMath::Clamp(1 - DistanceFromGround / MaxGroundDistance, 0, 1); 
			float AttractionStrength = -1 * FMath::Clamp(MaxAttractionStrength * StrengthMultiplier, MinAttractionStrength, MaxAttractionStrength);
			
			// AttractionStrength = -1 * FMath::Clamp(1/(DistanceFromGround + 1) * 300, 50, 300);
			UE_LOG(LogTemp, Warning, TEXT("FX comp, TickComponent, DST: %f, AttractionStrength %f, SpawnRate: %f"), DistanceFromGround, AttractionStrength, SpawnRateValue);
			
			TankHoverSmokeFX->SetFloatParameter(TEXT("AttractionStrength"), AttractionStrength);

			// set the TankHoverSmokeFX relative location to the ground
			FVector GroundLocation = TankMovementComponent->GetOwner()->GetActorLocation() - GroundSurfaceNormal * DistanceFromGround;
			GroundLocation.Z -= 50;
			TankHoverSmokeFX->SetWorldLocation(GroundLocation);
			
			// sample the color of the ground texture, and apply it to the smoke
			// FVector GroundColor = FVector(0.5, 0.5, 0.5);
		}
	}
}

void UHTTankEffectsComponent::BrakeLights(const bool bBraking) const
{

	if (TankLightsDynamicMaterialInstance)
	{
		const float Strength = bBraking ? TankLightsBrakeMaxStrength : TankLightsBrakeDefaultStrength;
		TankLightsDynamicMaterialInstance->SetScalarParameterValue(TankLightsBrakeStrengthName, Strength);
	}
}

void UHTTankEffectsComponent::ThrusterLights(const bool bThrusting) const
{
	if (TankLightsDynamicMaterialInstance)
	{
		const float Strength = bThrusting ? TankLightsThrusterMaxStrength : TankLightsThrusterDefaultStrength;
		TankLightsDynamicMaterialInstance->SetScalarParameterValue(TankLightsThrusterStrengthName, Strength);
	}
}

void UHTTankEffectsComponent::OnDeath()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("FX comp, OnDeath only on Authority"));
		return;
	}
	
	TeamColorEmissiveStrength = 0;
	OnRep_TeamColorEmissiveStrength();

	MulticastActivateBurningFX();

	bAreLightsOn = false;
	OnRep_LightsOn();
	
	// bIsBurningFxActive = true;
	// OnRep_IsBurningFxActive();
}

void UHTTankEffectsComponent::MulticastActivateBurningFX_Implementation()
{
	TankBurningFX->Activate();
}

// void UHTTankEffectsComponent::OnRep_IsBurningFxActive()
// {
// 	if (!TankBurningFx)
// 	{
// 		return;
// 	}
// 	
// 	if (bIsBurningFxActive)
// 	{
// 		TankBurningFx->Activate();
// 	}
// 	else
// 	{
// 		TankBurningFx->Deactivate();
// 	}
// }

void UHTTankEffectsComponent::OnRep_TeamColorEmissiveStrength()
{
	if (TankLightsDynamicMaterialInstance)
	{
		// UE_LOG(LogTemp, Warning, TEXT("FX comp, OnRep_TeamColorEmissiveStrength, strength %f"), TeamColorEmissiveStrength);
		TankLightsDynamicMaterialInstance->SetScalarParameterValue(TeamColorMaterialParamStrengthName, TeamColorEmissiveStrength);
	}
}

void UHTTankEffectsComponent::ApplyTeamColors(UHTTeamDataAsset* InTeamDataAsset)
{
	FString RoleString;
	UEnum::GetValueAsString(GetOwner()->GetLocalRole(), RoleString);
	
	if (!InTeamDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("FX comp, ApplyTeamColors, no team data asset, role %s"), *RoleString);
		return;
	}

	// UE_LOG(LogTemp, Warning, TEXT("FX comp, ApplyTeamColors, color %s, role %s"), *InTeamDataAsset->GetTeamShortName().ToString(), *RoleString);
	TeamDataAsset = InTeamDataAsset;
	OnRep_TeamDataAsset();
}


void UHTTankEffectsComponent::OnRep_TeamDataAsset()
{
	// UE_LOG(LogTemp, Warning, TEXT("FX comp, OnRep_TeamDataAsset, color %s"), *TeamDataAsset->GetTeamShortName().ToString());
	TeamDataAsset->ApplyToActor(GetOwner());
}

void UHTTankEffectsComponent::ServerToggleLights_Implementation()
{
	bAreLightsOn = !bAreLightsOn;
	OnRep_LightsOn();
}

void UHTTankEffectsComponent::OnRep_LightsOn()
{
	AHTHoverTank* HoverTank = Cast<AHTHoverTank>(GetOwner());
	if (!HoverTank)
	{
		return;
	}
	
	if (bAreLightsOn)
	{
		HoverTank->GetTankLights()->SetHiddenInGame(false);
		// TankBurningFx->Activate();
	}
	else
	{
		HoverTank->GetTankLights()->SetHiddenInGame(true);
		// TankBurningFx->Deactivate();
	}

	if (TankLightsDynamicMaterialInstance)
	{
		const FVector Color = TankLightsHeadlight;
		const float Strength = bAreLightsOn ? 100 : 0;
		TankLightsDynamicMaterialInstance->SetVectorParameterValue(TankLightsHeadlightColorName, Color);
		TankLightsDynamicMaterialInstance->SetScalarParameterValue(TankLightsHeadlightColorStrengthName, Strength);

		// log
		// FString RoleString;
		// UEnum::GetValueAsString(GetOwner()->GetLocalRole(), RoleString);
		// UE_LOG(LogTemp, Warning, TEXT("FX comp, OnRep_LightsOn, role %s, color %s, strength %f"), *RoleString, *Color.ToString(), Strength);
	}
}

