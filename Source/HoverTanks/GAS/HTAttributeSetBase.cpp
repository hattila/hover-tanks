// Fill out your copyright notice in the Description page of Project Settings.


#include "HTAttributeSetBase.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "HoverTanks/Controllers/HoverTankPlayerController.h"
#include "HoverTanks/Game/HTPlayerState.h"
#include "HoverTanks/Game/GameModes/HandlesTankDeathGameModeInterface.h"
#include "HoverTanks/Pawns/HoverTank.h"

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
	 * This is called before any modification happens.
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
		// UE_LOG(LogTemp, Warning, TEXT("UHTAttributeSetBase::PostGameplayEffectExecute: DamageAttribute changed, value %s"), *FString::SanitizeFloat(GetDamage()));
		
		// We have the calculated damage value now, subtract damage from shields first, calculate overflow of damage to health
		float LocalDamage = GetDamage();
		SetDamage(0.f);

		if (GetShield() > 0.f)
		{
			// Apply damage to shield first
			float DamageToShield = FMath::Min(GetShield(), LocalDamage);
			SetShield(GetShield() - DamageToShield);

			LocalDamage = LocalDamage - DamageToShield;
		}

		if (LocalDamage > 0.f)
		{
			SetHealth(GetHealth() - LocalDamage);
		}

		if (GetHealth() <= 0.f)
		{
			// UE_LOG(LogTemp, Warning, TEXT("UHTAttributeSetBase::PostGameplayEffectExecute: Health is 0, so the actor should die now."));
			
			AActor* Instigator = Data.EffectSpec.GetEffectContext().GetInstigator(); // This is a PlayerController
			AActor* EffectCauser = Data.EffectSpec.GetEffectContext().GetEffectCauser(); // an actor such as a projectile
			AController* SourceController = Instigator ? Cast<AController>(Instigator) : nullptr;

			// AActor* TargetActor = GetOwningAbilitySystemComponent()->GetOwner();
			// //
			// UE_LOG(LogTemp, Warning, TEXT("PostGameplayEffectExecute: EffectCauser: %s, \n SourceController %s, TargetActor: %s"),
			// 	// InstigatorPlayerState ? *InstigatorPlayerState->GetPlayerName() : TEXT("null"),
			// 	EffectCauser ? *EffectCauser->GetName() : TEXT("null"),
			// 	SourceController ? *SourceController->GetName() : TEXT("null"),
			// 	TargetActor ? *TargetActor->GetName() : TEXT("null")
			// 	);

			if (!SourceController)
			{
				UE_LOG(LogTemp, Warning, TEXT("UHTAttributeSetBase::PostGameplayEffectExecute: SourceController could not be detected, no OnOutOfHealth event will be broadcasted."));
				return;
			}

			OnOutOfHealth.Broadcast(SourceController, EffectCauser, Data.EffectSpec, Data.EvaluatedData.Magnitude);
		}
		return;
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
		return;
	}

	if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	{
		SetShield(FMath::Clamp(GetShield(), 0.0f, GetMaxShield()));
		return;
	}
}
