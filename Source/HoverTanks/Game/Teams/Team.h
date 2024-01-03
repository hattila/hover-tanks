// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Team.generated.h"

class UTeamDataAsset;

UCLASS()
class HOVERTANKS_API ATeam : public AInfo
{
	GENERATED_BODY()

public:
	ATeam();

	void SetTeamId(int8 NewTeamId) { TeamId = NewTeamId; }
	int8 GetTeamId() const { return TeamId; }
	
	void SetTeamDataAsset(TObjectPtr<UTeamDataAsset> NewTeamDataAsset) { TeamDataAsset = NewTeamDataAsset; }
	UTeamDataAsset* GetTeamDataAsset() const { return TeamDataAsset; }
	
protected:

	
private:
	int8 TeamId;

	UTeamDataAsset* TeamDataAsset;
};
