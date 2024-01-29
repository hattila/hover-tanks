// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTanksAssetManager.h"
#include "AbilitySystemGlobals.h"

void UHoverTanksAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	UAbilitySystemGlobals::Get().InitGlobalData();

	// log hello
	// UE_LOG(LogTemp, Warning, TEXT("Hello from UHoverTanksAssetManager::StartInitialLoading()"));
}
