// Fill out your copyright notice in the Description page of Project Settings.


#include "HTTeamDataAsset.h"

#include "NiagaraComponent.h"

void UHTTeamDataAsset::ApplyToActor(AActor* TargetActor, bool bIncludeChildActors)
{
	if (TargetActor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHTTeamDataAsset::ApplyToActor: TargetActor is nullptr"));
		return;
	}

	TargetActor->ForEachComponent(bIncludeChildActors, [=](UActorComponent* InComponent)
	{
		if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(InComponent))
		{
			// UE_LOG(LogTemp, Warning, TEXT("UHTTeamDataAsset::ApplyToActor: Applying to mesh component"));
			ApplyToMeshComponent(MeshComponent);
		}
		else if (UNiagaraComponent* NiagaraComponent = Cast<UNiagaraComponent>(InComponent))
		{
			// ApplyToNiagaraComponent(NiagaraComponent);
		}
	});		
}

void UHTTeamDataAsset::ApplyToMeshComponent(UMeshComponent* MeshComponent)
{

	const TArray<UMaterialInterface*> MaterialInterfaces = MeshComponent->GetMaterials();
	
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialInterfaces.Num(); ++MaterialIndex)
	{
		if (UMaterialInterface* MaterialInterface = MaterialInterfaces[MaterialIndex])
		{
			// apply to dynamic materials only
			UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(MaterialInterface);
			if (!DynamicMaterial)
			{
				// UE_LOG(LogTemp, Warning, TEXT("UHTTeamDataAsset::ApplyToMeshComponent: material is not yet dynamic"));
				DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(MaterialIndex);
				// continue;
			}

			for (const auto& ColorParameter : ColorParameters)
			{
				// UE_LOG(LogTemp, Warning, TEXT("UHTTeamDataAsset::ApplyToMeshComponent: setting parameter: %s with value %s"), *ColorParameter.Key.ToString(), *ColorParameter.Value.ToString());
				DynamicMaterial->SetVectorParameterValue(ColorParameter.Key, ColorParameter.Value);
			}
		}
	}
}
