// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Asset_GameplayAbility.generated.h"

class UAbilityInputBindingComponent;
class UInputAction;
class UGameplayAbility;

USTRUCT()
struct FGameplayAbilityInfo
{
	GENERATED_BODY()

	// UPROPERTY(EditAnywhere, Category = "GameplayAbility|BindInfo")
	// TSoftClassPtr<UGameplayAbility> GameplayAbilityClass;
	//
	// UPROPERTY(EditAnywhere, Category = "GameplayAbility|BindInfo")
	// TSoftObjectPtr<UInputAction> InputAction;

	// UPROPERTY(EditAnywhere, Category = "GameplayAbility|BindInfo")
	// TSoftClassPtr<UGameplayAbility> GameplayAbilityClass;
	//
	// UPROPERTY(EditAnywhere, Category = "GameplayAbility|BindInfo")
	// TSoftObjectPtr<UInputAction> InputAction;

	UPROPERTY(EditAnywhere, Category = "GameplayAbility|BindInfo")
	TSubclassOf<UGameplayAbility> GameplayAbilityClass;

	UPROPERTY(EditAnywhere, Category = "GameplayAbility|BindInfo")
	UInputAction* InputAction;
};

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UAsset_GameplayAbility : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "GameplayAbility|AbilitySet")
	TArray<FGameplayAbilityInfo> Abilities;

	// server
	// void GiveAbilities(UHTAbilitySystemComponent* AbilitySystemComponent) const;

	// server
	// void RemoteAbilities(UHTAbilitySystemComponent* AbilitySystemComponent) const;

	// local controller
	void BindAbility(UAbilityInputBindingComponent* AbilityInputBindingComponent, struct FGameplayAbilitySpec& Spec) const;

	// local controller
	void UnbindAbility(UAbilityInputBindingComponent* AbilityInputBindingComponent, struct FGameplayAbilitySpec& Spec) const;
};
