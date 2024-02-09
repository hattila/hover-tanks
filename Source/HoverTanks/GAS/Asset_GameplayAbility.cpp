// Fill out your copyright notice in the Description page of Project Settings.


#include "Asset_GameplayAbility.h"

#include "AbilityInputBindingComponent.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"

void UAsset_GameplayAbility::BindAbility(UAbilityInputBindingComponent* AbilityInputBindingComponent,
                                         FGameplayAbilitySpec& Spec) const
{
	check(Spec.Ability);
	check(AbilityInputBindingComponent);

	for (const FGameplayAbilityInfo& BindInfo : Abilities) 
	{
		if (BindInfo.GameplayAbilityClass == Spec.Ability->GetClass()) 
		{
			AbilityInputBindingComponent->SetInputBinding(BindInfo.InputAction, Spec);
		}
	}
}

void UAsset_GameplayAbility::UnbindAbility(UAbilityInputBindingComponent* AbilityInputBindingComponent, FGameplayAbilitySpec& Spec) const
{
	check(AbilityInputBindingComponent);
	AbilityInputBindingComponent->ClearInputBinding(Spec);
}
