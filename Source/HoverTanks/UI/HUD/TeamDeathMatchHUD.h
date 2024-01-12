// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DeathMatchHUD.h"
#include "CoreMinimal.h"
#include "TeamDeathMatchHUD.generated.h"

class UScoreBoardWidget;
class AHoverTankPlayerController;
class UHoverTankHUDWidget;
class ADeathMatchGameState;
class UDeathMatchPlayerHUDWidget;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API ATeamDeathMatchHUD : public ADeathMatchHUD
{
	GENERATED_BODY()

public:
	ATeamDeathMatchHUD();
};
