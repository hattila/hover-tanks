// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


class ADeathMatchGameMode;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOVERTANKS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure)
	float GetHealth() const { return Health; }

	// define an OnAnyDamage event
	UFUNCTION()
	void OnAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
	// void TakeDamage(float Damage);
	// void Heal(float HealAmount);

	bool IsDead() const { return Health <= 0.f; }
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	ADeathMatchGameMode* GameModeRef; // todo: respawn able game mode interface?
	
	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere)
	float Health = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float MaxHealth = 100.f;

	UFUNCTION()
	void OnRep_Health();

	bool IsOwningHoverTankDead();
};
