// Fill out your copyright notice in the Description page of Project Settings.


#include "HTPlayerState.h"

#include "HoverTanks/GAS/HTAbilitySystemComponent.h"
#include "HoverTanks/GAS/HTAttributeSetBase.h"

#include "AbilitySystemComponent.h"
#include "GameModes/HandlesTankDeathGameModeInterface.h"
#include "HoverTanks/Controllers/HoverTankPlayerController.h"
#include "HoverTanks/Pawns/HoverTank.h"
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

	DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
}

UAbilitySystemComponent* AHTPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool AHTPlayerState::IsDead()
{
	return AttributeSetBase->GetHealth() <= 0.0f;
}

void AHTPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHTAttributeSetBase::GetHealthAttribute())
			.AddUObject(this, &AHTPlayerState::OnHealthAttributeChangeHandler);

		AttributeSetBase->OnOutOfHealth.AddUObject(this, &AHTPlayerState::OnOutOfHealthHandler);
	}
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

void AHTPlayerState::OnHealthAttributeChangeHandler(const FOnAttributeChangeData& Data)
{
	// AHoverTank* HoverTank = Cast<AHoverTank>(GetPawn());
	// if (HoverTank && HoverTank->IsDead() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	// {
	// 	// log
	// 	UE_LOG(LogTemp, Warning, TEXT("AHTPlayerState::OnHealthAttributeChangeHandler Health: %f, had no dead tag, so it sould die now."), Data.NewValue);
	// }
}

void AHTPlayerState::OnOutOfHealthHandler(AController* InstigatorController, AActor* EffectCauser,
	const FGameplayEffectSpec& GameplayEffectSpec, float Magnitude)
{
	// log out every parameter
	UE_LOG(LogTemp, Warning, TEXT("AHTPlayerState::OnOutOfHealthHandler InstigatorController %s, EffectCauser %s, Magnitude %f"),
		InstigatorController ? *InstigatorController->GetName() : TEXT("null"),
		EffectCauser ? *EffectCauser->GetName() : TEXT("null"),
		Magnitude
	);

	if (!HasAuthority())
	{
		return;
	}

	// get the game mode
	IHandlesTankDeathGameModeInterface* HandlesTankDeathGameMode = Cast<IHandlesTankDeathGameModeInterface>(GetWorld()->GetAuthGameMode());
	if (HandlesTankDeathGameMode)
	{
		AHoverTank* Tank = Cast<AHoverTank>(GetPawn());
		if (Tank)
		{
			HandlesTankDeathGameMode->TankDies(Tank, InstigatorController);	
		}
		
	}
}
