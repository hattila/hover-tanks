// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HasScoreBoard.h"
#include "GameFramework/PlayerController.h"
#include "HoverTankPlayerController.generated.h"

class ADeathMatchGameMode;
class UDeathMatchScoreBoardWidget;
class UInGameMenu;
class UInputAction;
class UInputMappingContext;

/**
 * 
 */
UCLASS()
class HOVERTANKS_API AHoverTankPlayerController : public APlayerController, public IHasScoreBoard
{
	GENERATED_BODY()

public:
	AHoverTankPlayerController();

	UFUNCTION(Client, Unreliable)
	virtual void ClientOnScoresChanged() override;

	UFUNCTION(Client, Reliable)
	void ClientForceOpenScoreBoard(int32 TimeUntilRestartInSeconds);
	
	virtual void OnRep_Pawn() override; // Controller.cpp
	// virtual void OnRep_PlayerState() override; // Controller.cpp

protected:
	UFUNCTION()
	void ApplyTeamColorToPawn(int8 NewTeamId);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	
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
	UInGameMenu* InGameMenu;

	ADeathMatchGameMode* GameModeRef; // todo: respawn able game mode interface?
	
	void OpenInGameMenu();
	void OpenScoreBoard();
	void RequestRespawn();

	UFUNCTION(Server, Reliable)
	void ServerRequestRespawn();
};
