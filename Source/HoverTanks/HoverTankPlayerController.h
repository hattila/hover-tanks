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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPawnPossessedEvent, const FString&, InPawnClassName);

/**
 * 
 */
UCLASS()
class HOVERTANKS_API AHoverTankPlayerController : public APlayerController, public IHasScoreBoard
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "CustomEvents")
	FPawnPossessedEvent OnPawnPossessed;

	UPROPERTY(BlueprintAssignable, Category = "CustomEvents")
	FPawnPossessedEvent OnPawnUnPossessed;
	
	AHoverTankPlayerController();

	UFUNCTION(Client, Unreliable)
	virtual void ClientOnScoresChanged() override;

	UFUNCTION(Client, Reliable)
	void ClientForceOpenScoreBoard(int32 TimeUntilRestartInSeconds);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void OnTankDeathHandler();

	UFUNCTION(Client, Unreliable)
	void ClientAddPawnsHUDWidget(const FString& InPawnClassName);

	UFUNCTION(Client, Unreliable)
	void ClientRemovePawnsHUDWidget();
	
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
