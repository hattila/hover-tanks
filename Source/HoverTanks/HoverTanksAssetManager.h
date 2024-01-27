// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "HoverTanksAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHoverTanksAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	virtual void StartInitialLoading() override;
};
