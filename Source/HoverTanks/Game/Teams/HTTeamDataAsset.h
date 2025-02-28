// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HTTeamDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTANKS_API UHTTeamDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, FLinearColor> ColorParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TeamShortName;

	UFUNCTION(BlueprintCallable, Category = Teams)
	void ApplyToActor(AActor* TargetActor, bool bIncludeChildActors = true);

	FLinearColor GetTeamColor() const { return ColorParameters[TEXT("TeamColor")]; }
	FLinearColor GetTextColor() const { return ColorParameters[TEXT("TextColor")]; }

	FText GetTeamShortName() const { return TeamShortName; }
	
private:

	void ApplyToMeshComponent(UMeshComponent* MeshComponent);
};
