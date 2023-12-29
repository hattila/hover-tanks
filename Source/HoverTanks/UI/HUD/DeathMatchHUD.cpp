// Fill out your copyright notice in the Description page of Project Settings.

#include "DeathMatchHUD.h"

#include "HoverTankHUDWidget.h"
#include "HoverTanks/Game/DeathMatchGameState.h"
#include "HoverTanks/UI/HUD/DeathMatchPlayerHUDWidget.h"
#include "HoverTanks/UI/DeathMatchScoreBoardWidget.h"
#include "HoverTanks/Components/WeaponsComponent.h"

#include "Blueprint/UserWidget.h"
#include "HoverTanks/HoverTank.h"

ADeathMatchHUD::ADeathMatchHUD(): DeathMatchGameStateRef(nullptr),
                                  DeathMatchPlayerHUDWidget(nullptr),
                                  HoverTankHUDWidget(nullptr),
                                  DeathMatchScoreBoardWidget(nullptr)
{
	// get hold of the blueprint versions of the widgets
	static ConstructorHelpers::FClassFinder<UUserWidget> DeathMatchPlayerHUDWidgetClassFinder(
		TEXT("/Game/HoverTanks/UI/HUD/WBP_DeathMatchPlayerHUDWidget"));
	if (DeathMatchPlayerHUDWidgetClassFinder.Succeeded())
	{
		DeathMatchPlayerHUDWidgetClass = DeathMatchPlayerHUDWidgetClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> DeathMatchScoreBoardClassFinder(
		TEXT("/Game/HoverTanks/UI/WBP_DeathMatchScoreBoardWidget"));
	if (DeathMatchScoreBoardClassFinder.Succeeded())
	{
		DeathMatchScoreBoardClass = DeathMatchScoreBoardClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> HoverTankHUDWidgetClassFinder(
		TEXT("/Game/HoverTanks/UI/HUD/WBP_HoverTankHUDWidget"));
	if (HoverTankHUDWidgetClassFinder.Succeeded())
	{
		HoverTankHUDWidgetClass = HoverTankHUDWidgetClassFinder.Class;
	}
}

void ADeathMatchHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	APlayerController* PlayerController = Cast<APlayerController>(GetOwningPlayerController());
	if (PlayerController)
	{
		// Broadcast in Controller.cpp OnRep_Pawn
		PlayerController->OnPossessedPawnChanged.AddDynamic(this, &ADeathMatchHUD::OnPossessedPawnChangedHandler);
	}
}

void ADeathMatchHUD::ToggleScoreBoard()
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchHUD::ToggleScoreBoard"));
	
	if (!ensure(DeathMatchScoreBoardWidget != nullptr))
	{
		return;
	}

	if (DeathMatchScoreBoardWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		DeathMatchScoreBoardWidget->SetVisibility(ESlateVisibility::Hidden);
		DeathMatchPlayerHUDWidget->SetVisibility(ESlateVisibility::Visible);
		return;
	}
	
	ADeathMatchGameState* DeathMatchGameState = GetWorld()->GetGameState<ADeathMatchGameState>();
	if (DeathMatchGameState)
	{
		DeathMatchScoreBoardWidget->SetTimeLeft(DeathMatchGameState->GetTimeRemaining());
		DeathMatchScoreBoardWidget->RefreshPlayerScores(DeathMatchGameState->GetPlayerScores());
	}

	DeathMatchScoreBoardWidget->SetVisibility(ESlateVisibility::Visible);
	DeathMatchPlayerHUDWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ADeathMatchHUD::ForceOpenScoreBoard()
{
	if (!ensure(DeathMatchScoreBoardWidget != nullptr))
	{
		return;
	}

	if (DeathMatchGameStateRef)
	{
		DeathMatchScoreBoardWidget->SetTimeLeft(DeathMatchGameStateRef->GetTimeRemaining());
		DeathMatchScoreBoardWidget->RefreshPlayerScores(DeathMatchGameStateRef->GetPlayerScores());
	}

	DeathMatchScoreBoardWidget->SetVisibility(ESlateVisibility::Visible);
	DeathMatchPlayerHUDWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ADeathMatchHUD::RefreshPlayerScores()
{
	if (DeathMatchGameStateRef != nullptr)
	{
		DeathMatchScoreBoardWidget->RefreshPlayerScores(DeathMatchGameStateRef->GetPlayerScores());
	}
}

void ADeathMatchHUD::BeginPlay()
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

	if (!ensure(DeathMatchScoreBoardClass != nullptr))
	{
		return;
	}

	if (DeathMatchScoreBoardWidget == nullptr)
	{
		DeathMatchScoreBoardWidget = CreateWidget<UDeathMatchScoreBoardWidget>(GetOwningPlayerController(), DeathMatchScoreBoardClass);
		DeathMatchScoreBoardWidget->Setup();
		DeathMatchScoreBoardWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (HoverTankHUDWidgetClass == nullptr)
	{
		HoverTankHUDWidget = CreateWidget<UHoverTankHUDWidget>(GetOwningPlayerController(), HoverTankHUDWidgetClass);
	}
}

void ADeathMatchHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (DeathMatchPlayerHUDWidget)
	{
		DeathMatchPlayerHUDWidget->Teardown();
	}

	if (DeathMatchScoreBoardWidget)
	{
		DeathMatchScoreBoardWidget->Teardown();
	}

	if (HoverTankHUDWidget)
	{
		HoverTankHUDWidget->RemoveFromParent();
	}
}

void ADeathMatchHUD::OnPossessedPawnChangedHandler(APawn* OldPawn, APawn* NewPawn)
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
		HoverTank->OnTankDeath.AddDynamic(this, &ADeathMatchHUD::OnTankDeathHandler);
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

void ADeathMatchHUD::OnTankDeathHandler()
{
	// UE_LOG(LogTemp, Warning, TEXT("ADeathMatchHUD::OnTankDeathHandler"));
	
	if (HoverTankHUDWidget && HoverTankHUDWidget->IsInViewport())
	{
		HoverTankHUDWidget->RemoveFromParent();
	}
}