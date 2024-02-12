// Fill out your copyright notice in the Description page of Project Settings.


#include "HTPlayerState.h"

#include "HoverTanks/GAS/HTAbilitySystemComponent.h"
#include "HoverTanks/GAS/HTAttributeSetBase.h"

#include "AbilitySystemComponent.h"
#include "HoverTanks/Controllers/HoverTankPlayerController.h"
#include "HoverTanks/UI/HUD/HTPlayerHUD.h"

AHTPlayerState::AHTPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UHTAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// https://github.com/tranek/GASDocumentation?tab=readme-ov-file#441-attribute-set-definition
	AttributeSetBase = CreateDefaultSubobject<UHTAttributeSetBase>(TEXT("AttributeSetBase"));

	NetUpdateFrequency = 30.0f;

	// bind function to OnPawnSet
	OnPawnSet.AddDynamic(this, &AHTPlayerState::OnPawnChanged);
}

UAbilitySystemComponent* AHTPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AHTPlayerState::OnPawnChanged(APlayerState* PlayerState, APawn* NewPawn, APawn* OldPawn)
{
	APlayerController* PlayerController = Cast<AHoverTankPlayerController>(GetOwner());
	if (!PlayerController)
	{
		// log
		UE_LOG(LogTemp, Warning, TEXT("AHTPlayerState::OnPawnChanged PlayerController is null"));
		return;
	}
	
	AHTPlayerHUD* HUD = Cast<AHTPlayerHUD>(PlayerController->GetHUD());
	if (!HUD)
	{
		UE_LOG(LogTemp, Warning, TEXT("AHTPlayerState::OnPawnChanged DeathMatchHUD is null"));
		return;
	}

	HUD->OnPossessedPawnChangedHandler(OldPawn, NewPawn);
}
