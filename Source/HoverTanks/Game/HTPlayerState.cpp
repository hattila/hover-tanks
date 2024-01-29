// Fill out your copyright notice in the Description page of Project Settings.


#include "HTPlayerState.h"

#include "HoverTanks/GAS/HTAbilitySystemComponent.h"
#include "HoverTanks/GAS/HTAttributeSetBase.h"

#include "AbilitySystemComponent.h"

AHTPlayerState::AHTPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UHTAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UHTAttributeSetBase>(TEXT("AttributeSet"));

	NetUpdateFrequency = 30.0f;
}

UAbilitySystemComponent* AHTPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
