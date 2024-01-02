// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TeamDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UTeamDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, FLinearColor> ColorParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TeamShortName;

	UFUNCTION(BlueprintCallable, Category = Teams)
	void ApplyToActor(AActor* TargetActor, bool bIncludeChildActors = true);
private:

	void ApplyToMeshComponent(UMeshComponent* MeshComponent);
};
