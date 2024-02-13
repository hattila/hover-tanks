// Fill out your copyright notice in the Description page of Project Settings.


#include "HTPlayerHUD.h"

#include "HoverTankHUDWidget.h"
#include "HoverTanks/UI/HUD/DeathMatchPlayerHUDWidget.h"
#include "HoverTanks/UI/ScoreBoard/DeathMatchScoreBoardWidget.h"
#include "HoverTanks/Components/WeaponsComponent.h"
#include "HoverTanks/Game/GameStates/DeathMatchGameState.h"
#include "HoverTanks/GAS/HTAttributeSetBase.h"
#include "HoverTanks/Pawns/HoverTank.h"

#include "AbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"

AHTPlayerHUD::AHTPlayerHUD()
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

void AHTPlayerHUD::BeginPlay()
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

void AHTPlayerHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

void AHTPlayerHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	/**
	 * Instead of here, the PlayerState should be the one to listen for changes in the possessed pawn, bacouse the
	 * PlayerState is the one that is replicated to the client, and it has the Ability System Component.
	 */
	
	// APlayerController* PlayerController = Cast<APlayerController>(GetOwningPlayerController());
	// if (PlayerController)
	// {
	// 	// Broadcast in Controller.cpp OnRep_Pawn
	// 	PlayerController->OnPossessedPawnChanged.AddDynamic(this, &AHTPlayerHUD::OnPossessedPawnChangedHandler);
	// }
}

void AHTPlayerHUD::ToggleScoreBoard()
{
	// UE_LOG(LogTemp, Warning, TEXT("AHTPlayerHUD::ToggleScoreBoard"));
	
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

void AHTPlayerHUD::ForceOpenScoreBoard()
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

void AHTPlayerHUD::RefreshPlayerScores()
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

void AHTPlayerHUD::AddKillIndicator(const FString& KillerName, const FString& VictimName, FLinearColor KillerColor, FLinearColor VictimColor)
{
	if (PlayerHUDWidget == nullptr)
	{
		return;
	}

	PlayerHUDWidget->AddKillIndicator(KillerName, VictimName, KillerColor, VictimColor);
}

void AHTPlayerHUD::OnPossessedPawnChangedHandler(APawn* OldPawn, APawn* NewPawn)
{
	// log
	UE_LOG(LogTemp, Warning, TEXT("AHTPlayerHUD::OnPossessedPawnChangedHandler"));
	
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
		// UE_LOG(LogTemp, Warning, TEXT("HoverTank found! Adding event handlers to HUDWidget."));

		// HoverTank->OnTankHealthChange.AddDynamic(HoverTankHUDWidget, &UHoverTankHUDWidget::OnHealthChangeHandler);
		HoverTank->OnTankDeath.AddDynamic(this, &AHTPlayerHUD::OnTankDeathHandler);
		HoverTank->OnWeaponSwitched.AddDynamic(HoverTankHUDWidget, &UHoverTankHUDWidget::OnWeaponSwitchedHandler);

		if (HoverTank->GetWeaponsComponent() != nullptr)
		{
			HoverTank->GetWeaponsComponent()->OnWeaponFire.AddDynamic(HoverTankHUDWidget, &UHoverTankHUDWidget::OnWeaponFireHandler);
		}

		UAbilitySystemComponent* AbilitySystemComponent = HoverTank->GetAbilitySystemComponent();
		if (AbilitySystemComponent)
		{
			SetupAbilitySystemAttributeChangeHandlers(AbilitySystemComponent);
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

void AHTPlayerHUD::OnTankDeathHandler()
{
	if (HoverTankHUDWidget && HoverTankHUDWidget->IsInViewport())
	{
		HoverTankHUDWidget->RemoveFromParent();
	}

	if (PlayerHUDWidget && PlayerHUDWidget->IsInViewport())
	{
		PlayerHUDWidget->ShowRespawnTextBorder();
	}
}

AGameStateBase* AHTPlayerHUD::GetSafeGameState() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	return GetWorld()->GetGameState<AGameStateBase>();
}

void AHTPlayerHUD::SetupAbilitySystemAttributeChangeHandlers(UAbilitySystemComponent* AbilitySystemComponent)
{
	// listen for attribute changes on the AbilitySystemComponent
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHTAttributeSetBase::GetShieldAttribute())
		.AddUObject(HoverTankHUDWidget, &UHoverTankHUDWidget::OnShieldAttributeChangeHandler);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHTAttributeSetBase::GetMaxShieldAttribute())
		.AddUObject(HoverTankHUDWidget, &UHoverTankHUDWidget::OnMaxShieldAttributeChangeHandler);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHTAttributeSetBase::GetHealthAttribute())
		.AddUObject(HoverTankHUDWidget, &UHoverTankHUDWidget::OnHealthAttributeChangeHandler);
			
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHTAttributeSetBase::GetMaxHealthAttribute())
		.AddUObject(HoverTankHUDWidget, &UHoverTankHUDWidget::OnMaxHealthAttributeChangeHandler);

	// trigger attribute changes, clients don't get the initial default attribute set GE change
	const UHTAttributeSetBase* AttributeSet = AbilitySystemComponent->GetSet<UHTAttributeSetBase>();
	if (AttributeSet)
	{
		FOnAttributeChangeData Data;
		Data.NewValue = AttributeSet->GetHealth();
		Data.OldValue = AttributeSet->GetHealth();
		HoverTankHUDWidget->OnHealthAttributeChangeHandler(Data);

		Data.NewValue = AttributeSet->GetMaxHealth();
		Data.OldValue = AttributeSet->GetMaxHealth();
		HoverTankHUDWidget->OnMaxHealthAttributeChangeHandler(Data);

		Data.NewValue = AttributeSet->GetShield();
		Data.OldValue = AttributeSet->GetShield();
		HoverTankHUDWidget->OnShieldAttributeChangeHandler(Data);

		Data.NewValue = AttributeSet->GetMaxShield();
		Data.OldValue = AttributeSet->GetMaxShield();
		HoverTankHUDWidget->OnMaxShieldAttributeChangeHandler(Data);
	}
}
