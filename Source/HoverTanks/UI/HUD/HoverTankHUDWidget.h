// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HoverTankHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHoverTankHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;
};
