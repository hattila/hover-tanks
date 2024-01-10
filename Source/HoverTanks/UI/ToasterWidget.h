// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Throbber.h"
#include "ToasterWidget.generated.h"

class UThrobber;
/**
 * 
 */
UCLASS()
class HOVERTANKS_API UToasterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	bool Initialize() override;
	
	void SetMessage(const FString& Message) const { MessageTextBlock->SetText(FText::FromString(Message)); }

	void ShowLoadingInProgress(const bool bShow = true) const { LoadingInProgressThrobber->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden); }

private:
	UPROPERTY(meta = (BindWidget))
	UThrobber* LoadingInProgressThrobber = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageTextBlock = nullptr;

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	UWidgetAnimation* ShowAnimation;

	void HideDelayed();
};
