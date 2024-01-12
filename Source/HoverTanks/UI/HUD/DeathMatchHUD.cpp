// Fill out your copyright notice in the Description page of Project Settings.

#include "DeathMatchHUD.h"

#include "HoverTankHUDWidget.h"
#include "HoverTanks/UI/HUD/DeathMatchPlayerHUDWidget.h"
#include "HoverTanks/UI/ScoreBoard/DeathMatchScoreBoardWidget.h"
#include "HoverTanks/Components/WeaponsComponent.h"
#include "HoverTanks/Game/GameStates/DeathMatchGameState.h"

#include "Blueprint/UserWidget.h"
#include "HoverTanks/Pawns/HoverTank.h"

ADeathMatchHUD::ADeathMatchHUD()
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


void ADeathMatchHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (PlayerHUDWidgetClass == nullptr)
	{
		return;
	}

	PlayerHUDWidget = CreateWidget<UDeathMatchPlayerHUDWidget>(GetOwningPlayerController(), PlayerHUDWidgetClass);
	PlayerHUDWidget->Setup();

	const ITimerGameStateInterface* TimerGameState = Cast<ITimerGameStateInterface>(GetSafeGameState());
	if (TimerGameState && PlayerHUDWidget)
	{
		PlayerHUDWidget->SetTimeLeft(TimerGameState->GetTimeRemaining());
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

void ADeathMatchHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->Teardown();
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
	// UE_LOG(LogTemp, Warning, TEXT("ADeathMatchHUD::ToggleScoreBoard"));
	
	if (!ensure(ScoreBoardWidget != nullptr))
	{
		return;
	}

	if (ScoreBoardWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		ScoreBoardWidget->SetVisibility(ESlateVisibility::Hidden);
		ScoreBoardWidget->SetInputModeGameOnly();

		PlayerHUDWidget->SetVisibility(ESlateVisibility::Visible);
		HoverTankHUDWidget->SetVisibility(ESlateVisibility::Visible);
		return;
	}
	
	const ITimerGameStateInterface* TimerGameState = Cast<ITimerGameStateInterface>(GetSafeGameState());
	if (TimerGameState)
	{
		ScoreBoardWidget->SetTimeLeft(TimerGameState->GetTimeRemaining());
	}

	const IScoringGameStateInterface* ScoringGameState = Cast<IScoringGameStateInterface>(GetSafeGameState());
	if (ScoringGameState)
	{
		ScoreBoardWidget->RefreshPlayerScores(ScoringGameState->GetPlayerScores());
	}

	ScoreBoardWidget->SetVisibility(ESlateVisibility::Visible);
	ScoreBoardWidget->SetupInputModeGameAndUi();

	PlayerHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	HoverTankHUDWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ADeathMatchHUD::ForceOpenScoreBoard()
{
	if (!ensure(ScoreBoardWidget != nullptr))
	{
		return;
	}

	const ITimerGameStateInterface* TimerGameState = Cast<ITimerGameStateInterface>(GetSafeGameState());
	if (TimerGameState)
	{
		ScoreBoardWidget->SetTimeLeft(TimerGameState->GetTimeRemaining());
	}

	const IScoringGameStateInterface* ScoringGameState = Cast<IScoringGameStateInterface>(GetSafeGameState());
	if (ScoringGameState)
	{
		ScoreBoardWidget->RefreshPlayerScores(ScoringGameState->GetPlayerScores());
	}

	ScoreBoardWidget->SetVisibility(ESlateVisibility::Visible);
	PlayerHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	HoverTankHUDWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ADeathMatchHUD::RefreshPlayerScores()
{
	if (ScoreBoardWidget == nullptr)
	{
		return;
	}

	const IScoringGameStateInterface* ScoringGameState = Cast<IScoringGameStateInterface>(GetSafeGameState());
	if (ScoringGameState)
	{
		ScoreBoardWidget->RefreshPlayerScores(ScoringGameState->GetPlayerScores());
	}
}

void ADeathMatchHUD::AddKillIndicator(const FString& KillerName, const FString& VictimName, FLinearColor KillerColor, FLinearColor VictimColor)
{
	if (PlayerHUDWidget == nullptr)
	{
		return;
	}

	PlayerHUDWidget->AddKillIndicator(KillerName, VictimName, KillerColor, VictimColor);
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

	if (PlayerHUDWidget != nullptr)
	{
		PlayerHUDWidget->ShowRespawnTextBorder(false);	
	}
}

void ADeathMatchHUD::OnTankDeathHandler()
{
	// UE_LOG(LogTemp, Warning, TEXT("ADeathMatchHUD::OnTankDeathHandler"));
	
	if (HoverTankHUDWidget && HoverTankHUDWidget->IsInViewport())
	{
		HoverTankHUDWidget->RemoveFromParent();
	}

	if (PlayerHUDWidget && PlayerHUDWidget->IsInViewport())
	{
		PlayerHUDWidget->ShowRespawnTextBorder();
	}
}

AGameStateBase* ADeathMatchHUD::GetSafeGameState() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	return GetWorld()->GetGameState<AGameStateBase>();
}