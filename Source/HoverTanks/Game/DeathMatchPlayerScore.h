#pragma once

#include "CoreMinimal.h"

#include "DeathMatchPlayerScore.generated.h"

USTRUCT()
struct FDeathMatchPlayerScore
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	int32 Score;
};
