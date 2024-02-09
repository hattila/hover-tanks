// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "HoverTanks/GAS/HTAttributeSetBase.h"
#include "HTDamageEffectExecutionCalculation.generated.h"


// Declare the attributes to capture and define how we want to capture them from the Source and Target.
struct HTDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Shield);

	HTDamageStatics()
	{
		// Snapshot happens at time of GESpec creation

		// Capture optional Damage set on the damage GE as a CalculationModifier under the ExecutionCalculation
		DEFINE_ATTRIBUTE_CAPTUREDEF(UHTAttributeSetBase, Damage, Source, true);

		// Capture the Target's Shield. Don't snapshot.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UHTAttributeSetBase, Shield, Target, false);
	}
};

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTDamageEffectExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UHTDamageEffectExecutionCalculation();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
