// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamDataAsset.h"

#include "NiagaraComponent.h"

void UTeamDataAsset::ApplyToActor(AActor* TargetActor, bool bIncludeChildActors)
{
	if (TargetActor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTeamDataAsset::ApplyToActor: TargetActor is nullptr"));
		return;
	}

	TargetActor->ForEachComponent(bIncludeChildActors, [=](UActorComponent* InComponent)
	{
		if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(InComponent))
		{
			// UE_LOG(LogTemp, Warning, TEXT("UTeamDataAsset::ApplyToActor: Applying to mesh component"));
			ApplyToMeshComponent(MeshComponent);
		}
		else if (UNiagaraComponent* NiagaraComponent = Cast<UNiagaraComponent>(InComponent))
		{
			// ApplyToNiagaraComponent(NiagaraComponent);
		}
	});		
}

void UTeamDataAsset::ApplyToMeshComponent(UMeshComponent* MeshComponent)
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
				// UE_LOG(LogTemp, Warning, TEXT("UTeamDataAsset::ApplyToMeshComponent: material is not yet dynamic"));
				DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(MaterialIndex);
				// continue;
			}

			for (const auto& ColorParameter : ColorParameters)
			{
				DynamicMaterial->SetVectorParameterValue(ColorParameter.Key, ColorParameter.Value);
			}
		}
	}
}
