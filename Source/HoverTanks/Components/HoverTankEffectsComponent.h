// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HoverTankEffectsComponent.generated.h"

class UTeamDataAsset;
class UMovementReplicatorComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOVERTANKS_API UHoverTankEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHoverTankEffectsComponent();

	// replication
	// void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnTeamIdChanged(int8 NewTeamId);

	void ApplyTeamColors(UTeamDataAsset* TeamDataAsset);
	
protected:
	virtual void BeginPlay() override;

private:
	UMovementReplicatorComponent* MovementReplicatorComponent = nullptr;

	UMaterialInstanceDynamic* TankLightsDynamicMaterialInstance = nullptr;
	
	/**
	 * Lights, thrusters
	 */
	FName TankLightsThrusterStrengthName = TEXT("StrengthB");
	float TankLightsThrusterDefaultStrength = .5f;
	float TankLightsThrusterMaxStrength = 1000.f;

	/**
	 * Lights, brake
	 */
	FName TankLightsBrakeStrengthName = TEXT("StrengthR");
	float TankLightsBrakeDefaultStrength = 0.f;
	float TankLightsBrakeMaxStrength = 100.f;
	
	void BrakeLights(bool bBraking) const;
	void ThrusterLights(bool bThrusting) const;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyTeamColors(UTeamDataAsset* TeamDataAsset);
};