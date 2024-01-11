// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HasScoreBoardController.h"
#include "GameFramework/PlayerController.h"
#include "HoverTankPlayerController.generated.h"

class ICanRequestRespawnGameModeInterface;
class UDeathMatchScoreBoardWidget;
class UInGameMenu;
class UInputAction;
class UInputMappingContext;

/**
 * 
 */
UCLASS()
class HOVERTANKS_API AHoverTankPlayerController : public APlayerController, public IHasScoreBoardController
{
	GENERATED_BODY()

public:
	AHoverTankPlayerController();

	// ~IHasScoreBoard interface
	UFUNCTION(Client, Unreliable)
	virtual void ClientOnScoresChanged() override;

	UFUNCTION(Client, Reliable)
	virtual void ClientForceOpenScoreBoard(int32 TimeUntilRestartInSeconds) override;
	// ~IHasScoreBoard interface
	
	virtual void OnRep_Pawn() override; // Controller.cpp

	UFUNCTION(Server, Reliable)
	void ServerAttemptToJoinTeam(int8 TeamId);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	
	UFUNCTION()
	void ApplyTeamColorToPawn(int8 NewTeamId);

	UFUNCTION(Server, Reliable)
	void ServerRefreshMeOnTheScoreBoard(int8 NewTeamId);
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* HoverTankPlayerControllerInputContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* OpenInGameMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* OpenScoreBoardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RequestRespawnAction;
	
	TSubclassOf<UUserWidget> InGameMenuClass;

	UPROPERTY()
	UInGameMenu* InGameMenu = nullptr;
	
	void OpenInGameMenuActionStarted();
	void OpenScoreBoardActionStarted();
	void RequestRespawnActionStarted();

	UFUNCTION(Server, Reliable)
	void ServerRequestRespawn();
};
