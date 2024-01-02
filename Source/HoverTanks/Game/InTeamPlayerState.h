// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "InTeamPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API AInTeamPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetTeamId(const int8 NewTeamId) { MyTeamId = NewTeamId; }
	int8 GetTeamId() const { return MyTeamId; }
	
private:
	UPROPERTY(Replicated)
	int8 MyTeamId = INDEX_NONE;
};
