// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "InTeamPlayerState.generated.h"

// define a delegate with one param
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamIdChanged, int8, NewTeamId);

/**
 * 
 */
UCLASS()
class HOVERTANKS_API AInTeamPlayerState : public APlayerState
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
	UPROPERTY(ReplicatedUsing=OnRep_TeamId)
	int8 MyTeamId = INDEX_NONE;

	UFUNCTION()
	void OnRep_TeamId() const;

	UFUNCTION()
	void OnPawnSetCallback(APlayerState* Player, APawn* NewPawn, APawn* OldPawn);
};