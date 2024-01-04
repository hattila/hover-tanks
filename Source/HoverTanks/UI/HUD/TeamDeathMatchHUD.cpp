// Fill out your copyright notice in the Description page of Project Settings.

#include "TeamDeathMatchHUD.h"

#include "HoverTankHUDWidget.h"
#include "HoverTanks/UI/ScoreBoardWidget.h"
#include "HoverTanks/UI/HUD/DeathMatchPlayerHUDWidget.h"
#include "HoverTanks/Components/WeaponsComponent.h"

#include "Blueprint/UserWidget.h"
#include "HoverTanks/HoverTank.h"
#include "HoverTanks/Game/GameModes/DeathMatchGameState.h"

ATeamDeathMatchHUD::ATeamDeathMatchHUD():
  HoverTankHUDWidget(nullptr),
  ScoreBoardWidget(nullptr)
{
	// get hold of the blueprint versions of the widgets
	static ConstructorHelpers::FClassFinder<UUserWidget> DeathMatchPlayerHUDWidgetClassFinder(
		TEXT("/Game/HoverTanks/UI/HUD/WBP_DeathMatchPlayerHUDWidget"));
	if (DeathMatchPlayerHUDWidgetClassFinder.Succeeded())
	{
		DeathMatchPlayerHUDWidgetClass = DeathMatchPlayerHUDWidgetClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> ScoreBoardClassFinder(
		TEXT("/Game/HoverTanks/UI/WBP_TeamDeathMatchScoreBoardWidget"));
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

void ATeamDeathMatchHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	APlayerController* PlayerController = Cast<APlayerController>(GetOwningPlayerController());
	if (PlayerController)
	{
		// Broadcast in Controller.cpp OnRep_Pawn
		PlayerController->OnPossessedPawnChanged.AddDynamic(this, &ATeamDeathMatchHUD::OnPossessedPawnChangedHandler);
	}
}

void ATeamDeathMatchHUD::ToggleScoreBoard()
{
	UE_LOG(LogTemp, Warning, TEXT("ATeamDeathMatchHUD::ToggleScoreBoard"));
	
	if (!ensure(ScoreBoardWidget != nullptr))
	{
		return;
	}

	if (ScoreBoardWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		ScoreBoardWidget->SetVisibility(ESlateVisibility::Hidden);
		DeathMatchPlayerHUDWidget->SetVisibility(ESlateVisibility::Visible);
		return;
	}
	
	ADeathMatchGameState* DeathMatchGameState = GetWorld()->GetGameState<ADeathMatchGameState>();
	if (DeathMatchGameState)
	{
		ScoreBoardWidget->SetTimeLeft(DeathMatchGameState->GetTimeRemaining());
		ScoreBoardWidget->RefreshPlayerScores(DeathMatchGameState->GetPlayerScores());
	}

	ScoreBoardWidget->SetVisibility(ESlateVisibility::Visible);
	DeathMatchPlayerHUDWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ATeamDeathMatchHUD::ForceOpenScoreBoard()
{
	if (!ensure(ScoreBoardWidget != nullptr))
	{
		return;
	}

	if (DeathMatchGameStateRef)
	{
		ScoreBoardWidget->SetTimeLeft(DeathMatchGameStateRef->GetTimeRemaining());
		ScoreBoardWidget->RefreshPlayerScores(DeathMatchGameStateRef->GetPlayerScores());
	}

	ScoreBoardWidget->SetVisibility(ESlateVisibility::Visible);
	DeathMatchPlayerHUDWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ATeamDeathMatchHUD::RefreshPlayerScores()
{
	if (DeathMatchGameStateRef != nullptr)
	{
		ScoreBoardWidget->RefreshPlayerScores(DeathMatchGameStateRef->GetPlayerScores());
	}
}

void ATeamDeathMatchHUD::BeginPlay()
{
	Super::BeginPlay();

	DeathMatchGameStateRef = GetWorld()->GetGameState<ADeathMatchGameState>(); // a GS with scores, and timer
	
	if (DeathMatchPlayerHUDWidgetClass == nullptr)
	{
		return;
	}

	DeathMatchPlayerHUDWidget = CreateWidget<UDeathMatchPlayerHUDWidget>(GetOwningPlayerController(), DeathMatchPlayerHUDWidgetClass);
	DeathMatchPlayerHUDWidget->Setup();

	if (DeathMatchGameStateRef && DeathMatchPlayerHUDWidget)
	{
		DeathMatchPlayerHUDWidget->SetTimeLeft(DeathMatchGameStateRef->GetTimeRemaining());
	}

	if (!ensure(ScoreBoardClass != nullptr))
	{
		return;
	}

	if (ScoreBoardWidget == nullptr)
	{
		ScoreBoardWidget = CreateWidget<UScoreBoardWidget>(GetOwningPlayerController(), ScoreBoardClass);
		ScoreBoardWidget->Setup();
		ScoreBoardWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (HoverTankHUDWidgetClass == nullptr)
	{
		HoverTankHUDWidget = CreateWidget<UHoverTankHUDWidget>(GetOwningPlayerController(), HoverTankHUDWidgetClass);
	}
}

void ATeamDeathMatchHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (DeathMatchPlayerHUDWidget)
	{
		DeathMatchPlayerHUDWidget->Teardown();
	}

	if (ScoreBoardWidget)
	{
		ScoreBoardWidget->Teardown();
	}

	if (HoverTankHUDWidget)
	{
		HoverTankHUDWidget->RemoveFromParent();
	}
}

void ATeamDeathMatchHUD::OnPossessedPawnChangedHandler(APawn* OldPawn, APawn* NewPawn)
{
	// UE_LOG(LogTemp, Warning, TEXT("ADeathMatchHUD::OnPossessedPawnChangedHandler, OldPawn: %s, NewPawn: %s"), OldPawn ? *OldPawn->GetName() : TEXT("null"), NewPawn ? *NewPawn->GetName() : TEXT("null"));
	//
	// const APawn* PossessedPawn = GetOwningPlayerController()->GetPawn();
	// UE_LOG(LogTemp, Warning, TEXT("ADeathMatchHUD::OnPlayerPossessed \n Current pawn is %s"), PossessedPawn ? *PossessedPawn->GetName() : TEXT("null"));
	
	// teardown the old pawn's HUDWidget
	// setup the new pawn's HUDWidget

	if (NewPawn == nullptr)
	{
		if (HoverTankHUDWidget && HoverTankHUDWidget->IsInViewport())
		{
			HoverTankHUDWidget->RemoveFromParent();
		}

		return;
	}
	
	if (HoverTankHUDWidgetClass != nullptr)
	{
		HoverTankHUDWidget = CreateWidget<UHoverTankHUDWidget>(GetOwningPlayerController(), HoverTankHUDWidgetClass);
	}
	
	AHoverTank* HoverTank = Cast<AHoverTank>(NewPawn);
	if (HoverTank && HoverTankHUDWidget)
	{
		// UE_LOG(LogTemp, Warning, TEXT("HoverTank found! Adding OnTankHealthChange handler"));
				
		HoverTank->OnTankHealthChange.AddDynamic(HoverTankHUDWidget, &UHoverTankHUDWidget::OnHealthChangeHandler);
		HoverTank->OnTankDeath.AddDynamic(this, &ATeamDeathMatchHUD::OnTankDeathHandler);
		HoverTank->OnWeaponSwitched.AddDynamic(HoverTankHUDWidget, &UHoverTankHUDWidget::OnWeaponSwitchedHandler);

		if (HoverTank->GetWeaponsComponent() != nullptr)
		{
			HoverTank->GetWeaponsComponent()->OnWeaponFire.AddDynamic(HoverTankHUDWidget, &UHoverTankHUDWidget::OnWeaponFireHandler);
		}
	}

	if (HoverTankHUDWidget && !HoverTankHUDWidget->IsInViewport())
	{
		HoverTankHUDWidget->AddToViewport();
	}
}

void ATeamDeathMatchHUD::OnTankDeathHandler()
{
	// UE_LOG(LogTemp, Warning, TEXT("ADeathMatchHUD::OnTankDeathHandler"));
	
	if (HoverTankHUDWidget && HoverTankHUDWidget->IsInViewport())
	{
		HoverTankHUDWidget->RemoveFromParent();
	}
}