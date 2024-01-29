// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "HTPlayerState.generated.h"

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

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	UHTAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(Transient)
	UHTAttributeSetBase* AttributeSet;
	
};
