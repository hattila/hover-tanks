// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "HTTeam.generated.h"

class UHTTeamDataAsset;

UCLASS()
class HOVERTANKS_API AHTTeam : public AInfo
{
	GENERATED_BODY()

public:
	AHTTeam();

	void SetTeamId(int8 NewTeamId) { TeamId = NewTeamId; }
	int8 GetTeamId() const { return TeamId; }
	
	void SetTeamDataAsset(TObjectPtr<UHTTeamDataAsset> NewTeamDataAsset) { TeamDataAsset = NewTeamDataAsset; }
	UHTTeamDataAsset* GetTeamDataAsset() const { return TeamDataAsset; }
	
protected:

	
private:
	int8 TeamId;

	UHTTeamDataAsset* TeamDataAsset;
};
