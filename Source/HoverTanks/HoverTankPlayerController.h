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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, Unreliable)
	virtual void ServerOnScoresChanged(const TArray<FDeathMatchPlayerScore>& InPlayerScores) override;

	UFUNCTION(Client, Reliable)
	void ClientForceOpenScoreBoard();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;

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

	TSubclassOf<UUserWidget> DeathMatchScoreBoardClass; // TODO: change to ScoreBoardClass, which will have DM and TDM children
	UDeathMatchScoreBoardWidget* DeathMatchScoreBoardWidget;

	UPROPERTY(ReplicatedUsing=OnRep_PlayerScores)
	TArray<FDeathMatchPlayerScore> PlayerScores;

	ADeathMatchGameMode* GameModeRef; // todo: respawn able game mode interface?
	
	void OpenInGameMenu();
	void OpenScoreBoard();
	void RequestRespawn();

	UFUNCTION(Server, Reliable)
	void ServerRequestRespawn();

	UFUNCTION()
	void OnRep_PlayerScores() const;
};
