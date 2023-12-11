// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MultiplayerGameControls.generated.h"

USTRUCT()
struct FHostGameSettings
{
	GENERATED_BODY()

	UPROPERTY()
	FString MapName = "PrototypeMap";

	UPROPERTY()
	FString GameModeName = "DeathMatch";

	UPROPERTY()
	uint32 MaxPlayers = 4;
};

// This class does not need to be modified.
UINTERFACE()
class UMultiplayerGameControls : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOVERTANKS_API IMultiplayerGameControls
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(Exec)
	virtual void Host() = 0;

	UFUNCTION(Exec)
	virtual void Join(const FString& Address) = 0;

	virtual void HostGame(const FHostGameSettings& InHostGameSettings) = 0;
	virtual void RefreshServerList() = 0;
	virtual void JoinAvailableGame(uint32 Index) = 0;
};
