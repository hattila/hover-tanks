// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTankEffectsComponent.h"

#include "HoverTankMovementComponent.h"
#include "MovementReplicatorComponent.h"
#include "NiagaraComponent.h"
#include "Components/RectLightComponent.h"
#include "HoverTanks/HoverTank.h"
#include "HoverTanks/Game/Teams/TeamDataAsset.h"
#include "Net/UnrealNetwork.h"


UHoverTankEffectsComponent::UHoverTankEffectsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TankBurningFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Burning FX"));
	// BurningFX->SetupAttachment(RootComponent);
	TankBurningFX->SetAutoActivate(false);
	
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BurningEmitterAsset(TEXT("/Game/HoverTanks/Niagara/NS_Burning"));
	UNiagaraSystem* BurningEmitterObject = BurningEmitterAsset.Object;
	TankBurningFX->SetAsset(BurningEmitterObject);
	TankBurningFX->SetRelativeLocation(FVector(-140.f, 0.f, 40.f));
	
	TankBurningFX->SetIsReplicated(true);
}

void UHoverTankEffectsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHoverTankEffectsComponent, TeamDataAsset);
	DOREPLIFETIME(UHoverTankEffectsComponent, bAreLightsOn);
	DOREPLIFETIME(UHoverTankEffectsComponent, TeamColorEmissiveStrength);
	// DOREPLIFETIME(UHoverTankEffectsComponent, bIsBurningFxActive);
}

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

void UHoverTankEffectsComponent::ServerOnDeath_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("FX comp, ServerOnDeath"));
	
	TeamColorEmissiveStrength = 0;
	OnRep_TeamColorEmissiveStrength();

	MulticastActivateBurningFX();

	bAreLightsOn = false;
	OnRep_LightsOn();
	
	// bIsBurningFxActive = true;
	// OnRep_IsBurningFxActive();
}

void UHoverTankEffectsComponent::MulticastActivateBurningFX_Implementation()
{
	TankBurningFX->Activate();
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

	AHoverTank* HoverTank = Cast<AHoverTank>(GetOwner());
	if (HoverTank)
	{
		// TankBurningFX = HoverTank->GetBurningFXComponent();
		TankBurningFX->AttachToComponent(HoverTank->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}

	// log out the TeamColorEmissiveStrength value
	FString RoleString;
	UEnum::GetValueAsString(GetOwner()->GetLocalRole(), RoleString);
	UE_LOG(LogTemp, Warning, TEXT("FX comp, BeginPlay, role %s, TeamColorEmissiveStrength %f"), *RoleString, TeamColorEmissiveStrength);
}

// void UHoverTankEffectsComponent::OnRep_IsBurningFxActive()
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

void UHoverTankEffectsComponent::OnRep_TeamColorEmissiveStrength()
{
	if (TankLightsDynamicMaterialInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("FX comp, OnRep_TeamColorEmissiveStrength, strength %f"), TeamColorEmissiveStrength);
		TankLightsDynamicMaterialInstance->SetScalarParameterValue(TeamColorMaterialParamStrengthName, TeamColorEmissiveStrength);
	}
}

void UHoverTankEffectsComponent::ApplyTeamColors(UTeamDataAsset* InTeamDataAsset)
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


void UHoverTankEffectsComponent::OnRep_TeamDataAsset()
{
	UE_LOG(LogTemp, Warning, TEXT("FX comp, OnRep_TeamDataAsset, color %s"), *TeamDataAsset->GetTeamShortName().ToString());
	TeamDataAsset->ApplyToActor(GetOwner());
}

void UHoverTankEffectsComponent::ServerToggleLights_Implementation()
{
	bAreLightsOn = !bAreLightsOn;
	OnRep_LightsOn();
}

void UHoverTankEffectsComponent::OnRep_LightsOn()
{
	AHoverTank* HoverTank = Cast<AHoverTank>(GetOwner());
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

