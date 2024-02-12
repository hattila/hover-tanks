// Fill out your copyright notice in the Description page of Project Settings.


#include "HTAbilitySystemComponent.h"

#include "AbilityBindingInterface.h"

// Sets default values for this component's properties
UHTAbilitySystemComponent::UHTAbilitySystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHTAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

/**
 * @see https://vorixo.github.io/devtricks/gas-input/#introduction
 */
void UHTAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	// log out the given ability
	UE_LOG(LogTemp, Warning, TEXT("UHTAbilitySystemComponent::OnGiveAbility() - AbilitySpec.Ability = %s"), *AbilitySpec.Ability->GetName());

	// log out role
	// UE_LOG(LogTemp, Warning, TEXT("UHTAbilitySystemComponent::OnGiveAbility() - Role = %d"), GetOwnerRole());
	
	// const IAbilityBindingInterface* ABI = Cast<IAbilityBindingInterface>(GetAvatarActor_Direct());
	// if (ABI)
	// {
	// 	ABI->BindAbility(AbilitySpec);
	// }
	
	Super::OnGiveAbility(AbilitySpec);
}


// Called every frame
void UHTAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
