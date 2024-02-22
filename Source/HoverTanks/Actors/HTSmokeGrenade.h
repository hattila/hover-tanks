// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HTSmokeGrenade.generated.h"

class UNiagaraComponent;

UCLASS()
class HOVERTANKS_API AHTSmokeGrenade : public AActor
{
	GENERATED_BODY()

public:
	AHTSmokeGrenade();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BaseMesh;

	// create a niagara system for the smoke
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Effects", meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* SmokeEffect;

};
