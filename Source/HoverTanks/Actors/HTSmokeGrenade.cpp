// Fill out your copyright notice in the Description page of Project Settings.


#include "HTSmokeGrenade.h"

#include "NiagaraComponent.h"


AHTSmokeGrenade::AHTSmokeGrenade()
{
	bReplicates = true;

	InitialLifeSpan = 10.f;

	// initialize the static mesh
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	RootComponent = BaseMesh;
	BaseMesh->SetCollisionProfileName(TEXT("Projectile"));

	// initialize the particle system
	SmokeEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	SmokeEffect->SetupAttachment(RootComponent);
	SmokeEffect->SetAutoActivate(true);
}

void AHTSmokeGrenade::BeginPlay()
{
	Super::BeginPlay();
	
}
