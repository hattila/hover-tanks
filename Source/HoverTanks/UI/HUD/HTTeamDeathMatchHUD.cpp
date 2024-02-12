// Fill out your copyright notice in the Description page of Project Settings.

#include "HTTeamDeathMatchHUD.h"

#include "HoverTankHUDWidget.h"

#include "Blueprint/UserWidget.h"

AHTTeamDeathMatchHUD::AHTTeamDeathMatchHUD()
{
	// get hold of the blueprint versions of the widgets
	static ConstructorHelpers::FClassFinder<UUserWidget> DeathMatchPlayerHUDWidgetClassFinder(
		TEXT("/Game/HoverTanks/UI/HUD/WBP_DeathMatchPlayerHUDWidget"));
	if (DeathMatchPlayerHUDWidgetClassFinder.Succeeded())
	{
		PlayerHUDWidgetClass = DeathMatchPlayerHUDWidgetClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> ScoreBoardClassFinder(
		TEXT("/Game/HoverTanks/UI/Scoreboard/WBP_TeamDeathMatchScoreBoardWidget"));
	if (ScoreBoardClassFinder.Succeeded())
	{
		ScoreBoardClass = ScoreBoardClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> HoverTankHUDWidgetClassFinder(
		TEXT("/Game/HoverTanks/UI/HUD/WBP_HoverTankHUDWidget"));
	if (HoverTankHUDWidgetClassFinder.Succeeded())
	{
		HoverTankHUDWidgetClass = HoverTankHUDWidgetClassFinder.Class;
	}
}
