#pragma once

#include "CoreMinimal.h"

#include "PlayerScore.generated.h"

USTRUCT()
struct FPlayerScore
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	int32 Score;

	UPROPERTY()
	int8 TeamId;
};
