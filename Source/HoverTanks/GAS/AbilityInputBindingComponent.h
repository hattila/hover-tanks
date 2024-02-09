// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilityInputBindingComponent.generated.h"

struct FGameplayAbilitySpecHandle;
struct FGameplayAbilitySpec;
class UAbilitySystemComponent;
class UInputAction;


USTRUCT()
struct FAbilityInputBinding
{
	GENERATED_BODY()

	int32  InputID = 0;
	uint32 OnPressedHandle = 0;
	uint32 OnReleasedHandle = 0;
	TArray<FGameplayAbilitySpecHandle> BoundAbilitiesStack;
};

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOVERTANKS_API UAbilityInputBindingComponent : public UInputComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
	void SetInputBinding(UInputAction* InputAction, FGameplayAbilitySpec& AbilitySpec);

	UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
	void ClearInputBinding(FGameplayAbilitySpec& AbilitySpec);

	UEnhancedInputComponent* GetInputComponent() const { return InputComponent; }
	void SetInputComponent(UEnhancedInputComponent* InInputComponent) { InputComponent = InInputComponent; }

	
private:
	void RunAbilitySystemSetup();

	void OnAbilityInputPressed(UInputAction* InputAction);
	void OnAbilityInputReleased(UInputAction* InputAction);

	void RemoveEntry(UInputAction* InputAction);
	FGameplayAbilitySpec* FindAbilitySpec(FGameplayAbilitySpecHandle Handle);
	
	/** The bound input component. */
	UPROPERTY(transient)
	UEnhancedInputComponent* InputComponent;

private:
	UPROPERTY(transient)
	UAbilitySystemComponent* AbilityComponent;

	UPROPERTY(transient)
	TMap<UInputAction*, FAbilityInputBinding> MappedAbilities;
};
