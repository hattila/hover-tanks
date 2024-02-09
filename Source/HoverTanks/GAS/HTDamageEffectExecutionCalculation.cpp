// Fill out your copyright notice in the Description page of Project Settings.

#include "HTDamageEffectExecutionCalculation.h"

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
	
	
}
