// Fill out your copyright notice in the Description page of Project Settings.


#include "HTAttributeSetBase.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

void UHTAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHTAttributeSetBase, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHTAttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHTAttributeSetBase, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHTAttributeSetBase, MaxShield, COND_None, REPNOTIFY_Always);
}

void UHTAttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHTAttributeSetBase, Health, OldHealth);
}

void UHTAttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHTAttributeSetBase, MaxHealth, OldMaxHealth);
}

void UHTAttributeSetBase::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHTAttributeSetBase, Shield, OldShield);
}

void UHTAttributeSetBase::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHTAttributeSetBase, MaxShield, OldMaxShield);
}

void UHTAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	/**
	 * This is called before any modification happens. This is the place to clamp attributes or do any other kind of modification.
	 */

	// If max shield is changed, regenerate shields to full
	if (Attribute == GetMaxShieldAttribute())
	{
		// set the Shield Value to the current NewValue
		UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
		AbilityComp->ApplyModToAttributeUnsafe(GetShieldAttribute(), EGameplayModOp::Override, NewValue);
		return;
	}

	if (Attribute == GetMaxHealthAttribute())
	{
		// set the Health Value to the current NewValue
		UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
		AbilityComp->ApplyModToAttributeUnsafe(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
	}
}

void UHTAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	/**
	 * This only triggers after changes to the BaseValue of an Attribute from an instant GameplayEffect.
	 * This is a valid place to do more Attribute manipulation when they change from a GameplayEffect.
	 */

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		 // Applying the damage is done in the AttributeSets PostGameplayEffectExecute method
		 // We have the calculated damage value now.
		 // Application logic can be eg:
		 // - subtract damage from shields first, calculate overflow of damage to health
		 // - show damage number for the damage causer
		 // - if damaged actor (avatar) died, add reward (xp, gold, money etc) to the damage causer

		float LocalDamage = GetDamage();
		float DamageOverShield = LocalDamage;

		if (GetShield() > 0.f)
		{
			// Apply damage to shield first
			float DamageToShield = FMath::Min(GetShield(), LocalDamage);
			SetShield(GetShield() - DamageToShield);
			
			DamageOverShield = FMath::Abs(GetShield() - LocalDamage);
		}

		SetHealth(GetHealth() - DamageOverShield);
	}
}
