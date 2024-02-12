// Fill out your copyright notice in the Description page of Project Settings.

#include "HTDeathMatchHUD.h"

#include "AbilitySystemComponent.h"
#include "HoverTankHUDWidget.h"
#include "HoverTanks/UI/HUD/DeathMatchPlayerHUDWidget.h"
#include "HoverTanks/UI/ScoreBoard/DeathMatchScoreBoardWidget.h"
#include "HoverTanks/Components/WeaponsComponent.h"
#include "HoverTanks/Game/GameStates/DeathMatchGameState.h"

#include "Blueprint/UserWidget.h"
#include "HoverTanks/GAS/HTAttributeSetBase.h"
#include "HoverTanks/Pawns/HoverTank.h"

AHTDeathMatchHUD::AHTDeathMatchHUD()
{
	// get hold of the blueprint versions of the widgets
	static ConstructorHelpers::FClassFinder<UUserWidget> DeathMatchPlayerHUDWidgetClassFinder(
		TEXT("/Game/HoverTanks/UI/HUD/WBP_DeathMatchPlayerHUDWidget"));
	if (DeathMatchPlayerHUDWidgetClassFinder.Succeeded())
	{
		PlayerHUDWidgetClass = DeathMatchPlayerHUDWidgetClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> DeathMatchScoreBoardClassFinder(
		TEXT("/Game/HoverTanks/UI/Scoreboard/WBP_DeathMatchScoreBoardWidget"));
	if (DeathMatchScoreBoardClassFinder.Succeeded())
	{
		ScoreBoardClass = DeathMatchScoreBoardClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> HoverTankHUDWidgetClassFinder(
		TEXT("/Game/HoverTanks/UI/HUD/WBP_HoverTankHUDWidget"));
	if (HoverTankHUDWidgetClassFinder.Succeeded())
	{
		HoverTankHUDWidgetClass = HoverTankHUDWidgetClassFinder.Class;
	}
}
