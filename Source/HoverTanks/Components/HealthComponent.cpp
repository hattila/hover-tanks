// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

#include "HoverTanks/HoverTank.h"
#include "HoverTanks/Game/GameModes/HandlesTankDeathGameModeInterface.h"
#include "HoverTanks/Game/GameModes/DeathMatchGameMode.h"
#include "HoverTanks/Game/GameModes/TeamDeathMatchGameMode.h"

#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, Health);
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::OnAnyDamage);
	}
}

void UHealthComponent::OnAnyDamage(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	AController* InstigatedBy,
	AActor* DamageCauser
)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (Health <= 0)
	{
		return;
	}
	
	Health -= Damage;
	// UE_LOG(LogTemp, Warning, TEXT("DamageTaken! Damage: %f, Health left: %f, Actor: %s"), Damage, Health, *DamagedActor->GetName());

	AHoverTank* HoverTank = Cast<AHoverTank>(GetOwner());
	if (HoverTank == nullptr)
	{
		return;
	}

	IHandlesTankDeathGameModeInterface* HandlesTankDeathGameMode = Cast<IHandlesTankDeathGameModeInterface>(GetWorld()->GetAuthGameMode());
	if (Health <= 0 && HandlesTankDeathGameMode)
	{
		HandlesTankDeathGameMode->TankDies(HoverTank, InstigatedBy);
	}

	OnRep_Health();
}

void UHealthComponent::Heal(const float HealAmount)
{
	Health = FMath::Clamp(Health + HealAmount, 0.f, MaxHealth);
	OnRep_Health();
}

void UHealthComponent::OnRep_Health()
{
	AHoverTank* HoverTank = Cast<AHoverTank>(GetOwner());
	if (HoverTank == nullptr)
	{
		return;
	}

	HoverTank->OnTankHealthChange.Broadcast(Health, MaxHealth);
}
