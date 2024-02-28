// Fill out your copyright notice in the Description page of Project Settings.


#include "HTPlayerState.h"

#include "HoverTanks/GAS/HTAbilitySystemComponent.h"
#include "HoverTanks/GAS/HTAttributeSetBase.h"
#include "HoverTanks/Game/GameModes/HandlesTankDeathGameModeInterface.h"
#include "HoverTanks/Controllers/HTPlayerController.h"
#include "HoverTanks/Pawns/HoverTank.h"

#include "AbilitySystemComponent.h"


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
	AHTPlayerController* PlayerController = Cast<AHTPlayerController>(GetOwner());
	if (!PlayerController)
	{
		// UE_LOG(LogTemp, Warning, TEXT("AHTPlayerState::OnPawnChanged PlayerController is null"));
		return;
	}

	if (NewPawn != nullptr)
	{
		PlayerController->ClientCreateTankHUD(NewPawn);	// or CreatePawnSpecificHUD in the future
	}
}

void AHTPlayerState::OnHealthAttributeChangeHandler(const FOnAttributeChangeData& Data)
{

}

void AHTPlayerState::OnOutOfHealthHandler(AController* InstigatorController, AActor* EffectCauser,
	const FGameplayEffectSpec& GameplayEffectSpec, float Magnitude)
{
	// log out every parameter
	// UE_LOG(LogTemp, Warning, TEXT("AHTPlayerState::OnOutOfHealthHandler InstigatorController %s, EffectCauser %s, Magnitude %f"),
	// 	InstigatorController ? *InstigatorController->GetName() : TEXT("null"),
	// 	EffectCauser ? *EffectCauser->GetName() : TEXT("null"),
	// 	Magnitude
	// );

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
