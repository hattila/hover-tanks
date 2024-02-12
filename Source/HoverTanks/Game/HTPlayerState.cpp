// Fill out your copyright notice in the Description page of Project Settings.


#include "HTPlayerState.h"

#include "HoverTanks/GAS/HTAbilitySystemComponent.h"
#include "HoverTanks/GAS/HTAttributeSetBase.h"

#include "AbilitySystemComponent.h"
#include "HoverTanks/Controllers/HoverTankPlayerController.h"
#include "HoverTanks/UI/HUD/DeathMatchHUD.h"

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
	// get the PlayerController
	APlayerController* PlayerController = Cast<AHoverTankPlayerController>(GetOwner());
	if (!PlayerController)
	{
		// log
		UE_LOG(LogTemp, Warning, TEXT("AHTPlayerState::OnPawnChanged PlayerController is null"));
		return;
	}
	
	ADeathMatchHUD* DeathMatchHUD = Cast<ADeathMatchHUD>(PlayerController->GetHUD());

	if (DeathMatchHUD)
	{
		// log
		UE_LOG(LogTemp, Warning, TEXT("AHTPlayerState::OnPawnChanged calling DeathMatchHUD->OnPossessedPawnChangedHandler()"));
		DeathMatchHUD->OnPossessedPawnChangedHandler(OldPawn, NewPawn);
	}
	else
	{
		//log
		UE_LOG(LogTemp, Warning, TEXT("AHTPlayerState::OnPawnChanged DeathMatchHUD is null"));
	}
	
	
}
