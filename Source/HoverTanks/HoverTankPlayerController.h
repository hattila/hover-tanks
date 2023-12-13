// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HoverTankPlayerController.generated.h"

class UInGameMenu;
class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API AHoverTankPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	// constructor
	AHoverTankPlayerController();

protected:
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* HoverTankPlayerControllerInputContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* OpenInGameMenuAction;

	TSubclassOf<UUserWidget> InGameMenuClass;
	UInGameMenu* InGameMenu;
	
	void OpenInGameMenu();
};
