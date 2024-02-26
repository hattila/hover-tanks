// Fill out your copyright notice in the Description page of Project Settings.

#include "HTDamageEffectExecutionCalculation.h"

#include "HTAttributeSetBase.h"

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

static const HTDamageStatics& DamageStatics()
{
	static HTDamageStatics DStatics;
	return DStatics;
}

UHTDamageEffectExecutionCalculation::UHTDamageEffectExecutionCalculation()
{
	RelevantAttributesToCapture.Add(DamageStatics().DamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().ShieldDef);
}

void UHTDamageEffectExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);
	
	// in the future, armor class will be used to calculate damage reduction

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);
	
	FAggregatorEvaluateParameters EvaluationParameters;
	
	float Damage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageDef, EvaluationParameters, Damage);

	// Add SetByCaller damage if it exists
	Damage += FMath::Max<float>(Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), false, -1.0f), 0.0f);

	// UE_LOG(LogTemp, Warning, TEXT("UHTDamageEffectExecutionCalculation::Execute_Implementation(), captured damages is %s"), *FString::SanitizeFloat(Damage));
	
	if (Damage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, Damage)
		);
	}
}
