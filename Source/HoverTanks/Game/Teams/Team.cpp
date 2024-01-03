// Fill out your copyright notice in the Description page of Project Settings.


#include "Team.h"


// Sets default values
ATeam::ATeam()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	// NetPriority = 3.0f;
	SetReplicatingMovement(false);
}
