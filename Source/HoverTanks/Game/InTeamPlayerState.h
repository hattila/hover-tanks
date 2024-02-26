// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HTPlayerState.h"
#include "InTeamPlayerState.generated.h"

// define a delegate with one param
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamIdChanged, int8, NewTeamId);

/**
 * 
 */
UCLASS()
class HOVERTANKS_API AInTeamPlayerState : public AHTPlayerState
{
	GENERATED_BODY()

public:
	FOnTeamIdChanged OnTeamIdChanged;
	
	// constructor
	AInTeamPlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetTeamId(const int8 NewTeamId);
	int8 GetTeamId() const { return MyTeamId; }
	
private:
	UPROPERTY(Replicated)
	int8 MyTeamId = INDEX_NONE;

	// UFUNCTION()
	// void OnPawnSetCallback(APlayerState* Player, APawn* NewPawn, APawn* OldPawn); // gets called by the engine, handy.
};
