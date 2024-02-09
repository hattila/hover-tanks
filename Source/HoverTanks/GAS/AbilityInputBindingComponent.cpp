// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityInputBindingComponent.h"

#include "AbilitySystemGlobals.h"
#include "EnhancedInputComponent.h"
#include "HTAbilitySystemComponent.h"

namespace AbilityInputBindingComponent_Impl
{
	constexpr int32 InvalidInputID = 0;
	int32 IncrementingInputID = InvalidInputID;

	static int32 GetNextInputID()
	{
		return ++IncrementingInputID;
	}
}

void UAbilityInputBindingComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (IsValid(Owner))
	{
		if (Owner->InputComponent)
		{
			InputComponent = CastChecked<UEnhancedInputComponent>(Owner->InputComponent);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UAbilityInputBindingComponent::BeginPlay: Owner has no InputComponent"));	
		}
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("UAbilityInputBindingComponent::BeginPlay: Owner is null or has no InputComponent"));
		UE_LOG(LogTemp, Warning, TEXT("UAbilityInputBindingComponent::BeginPlay: Owner is null"));
	}
}

void UAbilityInputBindingComponent::OnRegister()
{
	Super::OnRegister();

	// UWorld* World = GetWorld();
	// APawn* MyOwner = GetPawn<APawn>();
	//
	// if (ensure(MyOwner) && World->IsGameWorld())

	AActor* Owner = GetOwner();
	if (IsValid(Owner))
	{
		if (Owner->InputComponent)
		{
			InputComponent = CastChecked<UEnhancedInputComponent>(Owner->InputComponent);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UAbilityInputBindingComponent::OnRegister: Owner has no InputComponent"));	
		}
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("UAbilityInputBindingComponent::BeginPlay: Owner is null or has no InputComponent"));
		UE_LOG(LogTemp, Warning, TEXT("UAbilityInputBindingComponent::OnRegister: Owner is null"));
	}

	// log
	UE_LOG(LogTemp, Warning, TEXT("UAbilityInputBindingComponent::OnRegister"));
}

void UAbilityInputBindingComponent::SetInputBinding(UInputAction* InputAction, FGameplayAbilitySpec& AbilitySpec)
{
	using namespace AbilityInputBindingComponent_Impl;

	if (AbilitySpec.InputID == InvalidInputID)
	{
		AbilitySpec.InputID = GetNextInputID();
	}

	FAbilityInputBinding* AbilityInputBinding = MappedAbilities.Find(InputAction);
	if (!AbilityInputBinding)
	{
		AbilityInputBinding = &MappedAbilities.Add(InputAction);
	}

	AbilityInputBinding->BoundAbilitiesStack.AddUnique(AbilitySpec.Handle);

	if (InputComponent)
	{
		// Pressed event
		if (AbilityInputBinding->OnPressedHandle == 0)
		{
			AbilityInputBinding->OnPressedHandle = InputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &UAbilityInputBindingComponent::OnAbilityInputPressed, InputAction).GetHandle();
		}

		// Released event
		if (AbilityInputBinding->OnReleasedHandle == 0)
		{
			AbilityInputBinding->OnReleasedHandle = InputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &UAbilityInputBindingComponent::OnAbilityInputReleased, InputAction).GetHandle();
		}
	}
	else
	{
		// log
		UE_LOG(LogTemp, Warning, TEXT("UAbilityInputBindingComponent::SetInputBinding: InputComponent is null"));
	}
}

void UAbilityInputBindingComponent::ClearInputBinding(FGameplayAbilitySpec& AbilitySpec)
{
	using namespace AbilityInputBindingComponent_Impl;

	TArray<UInputAction*> InputActionsToClear;
	for (auto& InputBinding : MappedAbilities)
	{
		if (InputBinding.Value.BoundAbilitiesStack.Find(AbilitySpec.Handle))
		{
			InputActionsToClear.Add(InputBinding.Key);
		}
	}

	for (UInputAction* InputAction : InputActionsToClear)
	{
		FAbilityInputBinding* AbilityInputBinding = MappedAbilities.Find(InputAction);
		if (AbilityInputBinding->BoundAbilitiesStack.Remove(AbilitySpec.Handle) > 0)
		{
			if (AbilityInputBinding->BoundAbilitiesStack.Num() == 0)
			{
				// NOTE: This will invalidate the `AbilityInputBinding` ref above
				RemoveEntry(InputAction);
			}
		}
	}

	AbilitySpec.InputID = InvalidInputID;
}


void UAbilityInputBindingComponent::RunAbilitySystemSetup()
{
	AActor* MyOwner = GetOwner();
	check(MyOwner);

	AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MyOwner);
	if (AbilityComponent)
	{
		for (auto& InputBinding : MappedAbilities)
		{
			const int32 NewInputID = AbilityInputBindingComponent_Impl::GetNextInputID();
			InputBinding.Value.InputID = NewInputID;

			for (FGameplayAbilitySpecHandle AbilityHandle : InputBinding.Value.BoundAbilitiesStack)
			{
				FGameplayAbilitySpec* FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(AbilityHandle);
				if (FoundAbility != nullptr)
				{
					FoundAbility->InputID = NewInputID;
				}
			}
		}
	}
}


void UAbilityInputBindingComponent::OnAbilityInputPressed(UInputAction* InputAction)
{
	// The AbilitySystemComponent may not have been valid when we first bound input... try again.
	if (!AbilityComponent)
	{
		RunAbilitySystemSetup();
	}

	if (AbilityComponent)
	{
		using namespace AbilityInputBindingComponent_Impl;

		FAbilityInputBinding* FoundBinding = MappedAbilities.Find(InputAction);
		if (FoundBinding)
		{
			for (FGameplayAbilitySpecHandle AbilityHandle : FoundBinding->BoundAbilitiesStack)
			{
				FGameplayAbilitySpec* FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(AbilityHandle);
				if (FoundAbility != nullptr && ensure(FoundAbility->InputID != InvalidInputID))
				{
					AbilityComponent->AbilityLocalInputPressed(FoundAbility->InputID);
				}
			}
		}
	}
}

void UAbilityInputBindingComponent::OnAbilityInputReleased(UInputAction* InputAction)
{
	if (AbilityComponent)
	{
		using namespace AbilityInputBindingComponent_Impl;

		FAbilityInputBinding* FoundBinding = MappedAbilities.Find(InputAction);
		if (FoundBinding)
		{
			for (FGameplayAbilitySpecHandle AbilityHandle : FoundBinding->BoundAbilitiesStack)
			{
				FGameplayAbilitySpec* FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(AbilityHandle);
				if (FoundAbility != nullptr && ensure(FoundAbility->InputID != InvalidInputID))
				{
					AbilityComponent->AbilityLocalInputReleased(FoundAbility->InputID);
				}
			}
		}
	}
}

void UAbilityInputBindingComponent::RemoveEntry(UInputAction* InputAction)
{
	if (FAbilityInputBinding* Bindings = MappedAbilities.Find(InputAction))
	{
		if (InputComponent)
		{
			InputComponent->RemoveBindingByHandle(Bindings->OnPressedHandle);
			InputComponent->RemoveBindingByHandle(Bindings->OnReleasedHandle);
		}

		for (FGameplayAbilitySpecHandle AbilityHandle : Bindings->BoundAbilitiesStack)
		{
			using namespace AbilityInputBindingComponent_Impl;

			FGameplayAbilitySpec* AbilitySpec = FindAbilitySpec(AbilityHandle);
			if (AbilitySpec && AbilitySpec->InputID == Bindings->InputID)
			{
				AbilitySpec->InputID = InvalidInputID;
			}
		}

		MappedAbilities.Remove(InputAction);
	}
}

FGameplayAbilitySpec* UAbilityInputBindingComponent::FindAbilitySpec(FGameplayAbilitySpecHandle Handle)
{
	FGameplayAbilitySpec* FoundAbility = nullptr;
	if (AbilityComponent)
	{
		FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(Handle);
	}
	return FoundAbility;
}
