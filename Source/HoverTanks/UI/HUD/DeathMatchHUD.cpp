// Fill out your copyright notice in the Description page of Project Settings.

#include "DeathMatchHUD.h"

#include "HoverTankHUDWidget.h"
#include "HoverTanks/Game/DeathMatchGameState.h"
#include "HoverTanks/UI/HUD/DeathMatchPlayerHUDWidget.h"
#include "HoverTanks/UI/DeathMatchScoreBoardWidget.h"

#include "Blueprint/UserWidget.h"
#include "HoverTanks/HoverTank.h"
#include "HoverTanks/HoverTankPlayerController.h"


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

	// HasExposedPossessionEventsInterface?
	AHoverTankPlayerController* HoverTankPlayerController = Cast<AHoverTankPlayerController>(GetOwningPlayerController());
	if (HoverTankPlayerController)
	{
		HoverTankPlayerController->OnPawnPossessed.AddDynamic(this, &ADeathMatchHUD::OnPawnPossessedHandler);
		HoverTankPlayerController->OnPawnUnPossessed.AddDynamic(this, &ADeathMatchHUD::OnPawnUnPossessedHandler);
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

void ADeathMatchHUD::OnPawnPossessedHandler(const FString& InPawnClassName)
{
	// UE_LOG(LogTemp, Warning, TEXT("ADeathMatchHUD::OnPlayerPossessed"));

	/**
	 * Does the PossessedPawn have a HUDWidget? Which HUDWidget?
	 * Create and add it if so.
	 *
	 * HasOwnHUDWidgetInterface?
	 */

	if (InPawnClassName != TEXT("BP_HoverTank_C"))
	{
		UE_LOG(LogTemp, Warning, TEXT("Possessed pawn is not a HoverTank. it is %s"), *InPawnClassName);
		return;
	}
	
	if (HoverTankHUDWidgetClass != nullptr)
	{
		HoverTankHUDWidget = CreateWidget<UHoverTankHUDWidget>(GetOwningPlayerController(), HoverTankHUDWidgetClass);
	}
	
	if (HoverTankHUDWidget && !HoverTankHUDWidget->IsInViewport())
	{
		HoverTankHUDWidget->AddToViewport();
	}
}

void ADeathMatchHUD::OnPawnUnPossessedHandler(const FString& InPawnClassName)
{
	// UE_LOG(LogTemp, Warning, TEXT("ADeathMatchHUD::OnPlayerUnPossessed"));

	if (HoverTankHUDWidget && HoverTankHUDWidget->IsInViewport())
	{
		HoverTankHUDWidget->RemoveFromParent();
	}
}
