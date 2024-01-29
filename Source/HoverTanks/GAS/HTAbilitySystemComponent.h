// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "HTAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOVERTANKS_API UHTAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UHTAbilitySystemComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;


public:
	bool bCharacterAbilitiesGiven = false;
	bool bStartupEffectsApplied = false;
	
};
