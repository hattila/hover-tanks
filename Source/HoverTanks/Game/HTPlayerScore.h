#pragma once

#include "CoreMinimal.h"

#include "HTPlayerScore.generated.h"

USTRUCT()
struct FHTPlayerScore
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	int32 Score;

	UPROPERTY()
	int8 TeamId;
};
