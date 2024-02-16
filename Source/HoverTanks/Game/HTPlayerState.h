// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "HTPlayerState.generated.h"

struct FOnAttributeChangeData;
class UHTAbilitySystemComponent;
class UHTAttributeSetBase;

/**
 * 
 */
UCLASS()
class HOVERTANKS_API AHTPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// constructor
	AHTPlayerState();

	//~ IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ IAbilitySystemInterface

	UHTAttributeSetBase* GetAttributeSetBase() const { return AttributeSetBase; }
	bool IsDead();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	UHTAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(Transient)
	UHTAttributeSetBase* AttributeSetBase;

	FGameplayTag DeadTag;
	
	void BeginPlay() override;

private:
	UFUNCTION()
	void OnPawnChanged(APlayerState* PlayerState, APawn* NewPawn, APawn* OldPawn);

	virtual void OnHealthAttributeChangeHandler(const FOnAttributeChangeData& Data);

	void OnOutOfHealthHandler(AController* InstigatorController, AActor* EffectCauser, const FGameplayEffectSpec& GameplayEffectSpec, float Magnitude);
};
