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
	
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BurningEmitterAsset(TEXT("/Game/HoverTanks/Niagara/NS_Burning"));
	UNiagaraSystem* BurningEmitterObject = BurningEmitterAsset.Object;
	TankBurningFX->SetAsset(BurningEmitterObject);
	TankBurningFX->SetRelativeLocation(FVector(-140.f, 0.f, 40.f));
	
	TankBurningFX->SetIsReplicated(true);
}

void UHTTankEffectsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHTTankEffectsComponent, TeamDataAsset);
	DOREPLIFETIME(UHTTankEffectsComponent, bAreLightsOn);
	DOREPLIFETIME(UHTTankEffectsComponent, TeamColorEmissiveStrength);
	// DOREPLIFETIME(UHTTankEffectsComponent, bIsBurningFxActive);
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

void UHTTankEffectsComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementReplicatorComponent = GetOwner()->FindComponentByClass<UHTMovementReplicatorComponent>();

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
		TankBurningFX->AttachToComponent(HoverTank->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}

	// FString RoleString;
	// UEnum::GetValueAsString(GetOwner()->GetLocalRole(), RoleString);
	// UE_LOG(LogTemp, Warning, TEXT("FX comp, BeginPlay, role %s, TeamColorEmissiveStrength %f"), *RoleString, TeamColorEmissiveStrength);
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

