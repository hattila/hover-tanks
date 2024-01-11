// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HoverTankEffectsComponent.generated.h"

class UNiagaraComponent;
class UTeamDataAsset;
class UMovementReplicatorComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOVERTANKS_API UHoverTankEffectsComponent : public UActorComponent // USceneComponent
{
	GENERATED_BODY()

public:
	UHoverTankEffectsComponent();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ApplyTeamColors(UTeamDataAsset* InTeamDataAsset);

	UFUNCTION(Server, Reliable)
	void ServerToggleLights();

	void OnDeath();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastActivateBurningFX();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UMovementReplicatorComponent* MovementReplicatorComponent = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* TankLightsDynamicMaterialInstance = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* TankBurningFX = nullptr;
	
	// UPROPERTY(ReplicatedUsing=OnRep_IsBurningFxActive)
	// bool bIsBurningFxActive = false;
	//
	// UFUNCTION()
	// void OnRep_IsBurningFxActive();
	
	/**
	 * Team Color
	 */
	FName TeamColorMaterialParamName = TEXT("EmissiveTeamColor");
	FName TeamColorMaterialParamStrengthName = TEXT("EmissiveTeamColorStrength");

	UPROPERTY(ReplicatedUsing=OnRep_TeamColorEmissiveStrength)
	float TeamColorEmissiveStrength = 50.f;

	UFUNCTION()
	void OnRep_TeamColorEmissiveStrength();
	
	/**
	 * Lights, thrusters
	 */
	FName TankLightsThrusterStrengthName = TEXT("ThrusterColorStrength");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lights", meta = (AllowPrivateAccess = "true"))
	float TankLightsThrusterDefaultStrength = .5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lights", meta = (AllowPrivateAccess = "true"))
	float TankLightsThrusterMaxStrength = 1000.f;

	/**
	 * Lights, brake
	 */
	FName TankLightsBrakeStrengthName = TEXT("MiscColorStrength");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lights", meta = (AllowPrivateAccess = "true"))
	float TankLightsBrakeDefaultStrength = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lights", meta = (AllowPrivateAccess = "true"))
	float TankLightsBrakeMaxStrength = 100.f;

	/**
	 * Lights, headlights
	 */
	FName TankLightsHeadlightColorName = TEXT("LightsColor");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lights", meta = (AllowPrivateAccess = "true"))
	FVector TankLightsHeadlight = FVector(1.f, 1.f, 1.f);

	FName TankLightsHeadlightColorStrengthName = TEXT("LightsColorStrength");
	
	void BrakeLights(bool bBraking) const;
	void ThrusterLights(bool bThrusting) const;

	UPROPERTY(ReplicatedUsing=OnRep_LightsOn)
	bool bAreLightsOn = false;

	UFUNCTION()
	void OnRep_LightsOn();

	/**
	 * Team colors
	 */
	UPROPERTY(ReplicatedUsing=OnRep_TeamDataAsset)
	UTeamDataAsset* TeamDataAsset = nullptr;

	UFUNCTION()
	void OnRep_TeamDataAsset();
};
