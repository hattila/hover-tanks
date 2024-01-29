// Fill out your copyright notice in the Description page of Project Settings.


#include "HTAttributeSetBase.h"

#include "Net/UnrealNetwork.h"

void UHTAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHTAttributeSetBase, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHTAttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHTAttributeSetBase, Ammo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHTAttributeSetBase, MaxAmmo, COND_None, REPNOTIFY_Always);
}

void UHTAttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHTAttributeSetBase, Health, OldHealth);
}

void UHTAttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHTAttributeSetBase, MaxHealth, OldMaxHealth);
}

void UHTAttributeSetBase::OnRep_Ammo(const FGameplayAttributeData& OldAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHTAttributeSetBase, Ammo, OldAmmo);
}

void UHTAttributeSetBase::OnRep_MaxAmmo(const FGameplayAttributeData& OldMaxAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHTAttributeSetBase, MaxAmmo, OldMaxAmmo);
}
