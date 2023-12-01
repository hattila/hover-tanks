// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

#include "HoverTanks/Game/DeathMatchGameMode.h"
#include "HoverTanks/HoverTank.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// define the GetLifetimeReplicatedProps function
void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, Health);
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::OnAnyDamage);
	}

	// todo: respawn able game mode interface?
	GameModeRef = Cast<ADeathMatchGameMode>(GetWorld()->GetAuthGameMode());
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComponent::OnAnyDamage(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	AController* InstigatedBy,
	AActor* DamageCauser
)
{
	if (GetOwner()->HasAuthority())
	{
		Health -= Damage;
		UE_LOG(LogTemp, Warning, TEXT("DamageTaken! Damage: %f, Health left: %f, Actor: %s"), Damage, Health, *DamagedActor->GetName());

		if (Health <= 0 && GameModeRef)
		{
			AHoverTank* Tank = Cast<AHoverTank>(GetOwner());
			GameModeRef->TankDies(Tank);
		}
	}
}

void UHealthComponent::OnRep_Health()
{
	// update hud
	// UE_LOG(LogTemp, Warning, TEXT("Health changed! Health: %f"), Health);
}