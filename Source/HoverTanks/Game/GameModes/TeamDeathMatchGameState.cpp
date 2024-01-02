// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamDeathMatchGameState.h"

#include "HoverTanks/Game/InTeamPlayerState.h"
#include "HoverTanks/Game/Teams/Team.h"
#include "HoverTanks/Game/Teams/TeamDataAsset.h"

ATeamDeathMatchGameState::ATeamDeathMatchGameState()
{
	// find and set the team data asset
	static ConstructorHelpers::FObjectFinder<UTeamDataAsset> Team1DataAssetFinder(TEXT("/Game/HoverTanks/Game/Teams/DA_RedTeam"));
	if (Team1DataAssetFinder.Succeeded())
	{
		TeamsToCreate.Add(1, Team1DataAssetFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UTeamDataAsset> Team2DataAssetFinder(TEXT("/Game/HoverTanks/Game/Teams/DA_BlueTeam"));
	if (Team2DataAssetFinder.Succeeded())
	{
		TeamsToCreate.Add(2, Team2DataAssetFinder.Object);
	}
	
}

void ATeamDeathMatchGameState::CreateTeams()
{
	if (TeamMap.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Teams already created"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Creating Teams"));

	// iterate over TeamsToCreate
	for (const auto& TeamToCreate : TeamsToCreate)
	{
		// create a new team
		ATeam* NewTeam = GetWorld()->SpawnActor<ATeam>(ATeam::StaticClass());
		// set the team name
		NewTeam->SetTeamId(TeamToCreate.Key);
		NewTeam->SetTeamDataAsset(TeamToCreate.Value);

		TeamMap.Add(TeamToCreate.Key, NewTeam);
		// Teams.Add(NewTeam);
	}
}

void ATeamDeathMatchGameState::AssignPlayersToTeams()
{
	UE_LOG(LogTemp, Warning, TEXT("AssignPlayersToTeams"));
	
	// for each connected player, cast their player state to InTeamPlayerState, and assign them to a team
	for (APlayerState* PS : PlayerArray)
	{
		if (AInTeamPlayerState* InTeamPS = Cast<AInTeamPlayerState>(PS))
		{
			AssignPlayerToTeam(InTeamPS);
		}
	}
}

void ATeamDeathMatchGameState::AssignPlayerToTeam(AInTeamPlayerState* PlayerState)
{
	PlayerState->SetTeamId(GetLeastPopulatedTeamId());
}

int8 ATeamDeathMatchGameState::GetLeastPopulatedTeamId() const
{
	const int32 TeamsNum = TeamMap.Num();
	if (TeamsNum <= 0)
	{
		// UE_LOG(LogTemp, Warning, TEXT("GetLeastPopulatedTeamId : No teams created"));
		return INDEX_NONE;
	}

	TMap<int8, uint32> TeamMemberCounts;
	TeamMemberCounts.Reserve(TeamsNum);
	
	for (const auto& TeamElement : TeamMap)
	{
		// const int8 TeamId = TeamElement.Value->GetTeamId();
		const int8 TeamId = TeamElement.Key;
		TeamMemberCounts.Add(TeamId, 0);
	}

	// UE_LOG(LogTemp, Warning, TEXT("GetLeastPopulatedTeamId : TeamMemberCounts.Num() = %d"), TeamMemberCounts.Num());

	// for all connected players cast their player state to InTeamPlayerState, check team id, and increment the count for that team
	for (APlayerState* PS : PlayerArray)
	{
		if (const AInTeamPlayerState* InTeamPS = Cast<AInTeamPlayerState>(PS))
		{
			const int8 PlayerTeamId = InTeamPS->GetTeamId();
			if (PlayerTeamId != INDEX_NONE)
			{
				// UE_LOG(LogTemp, Warning, TEXT("GetLeastPopulatedTeamId : PlayerTeamId = %d"), PlayerTeamId);
				check(TeamMemberCounts.Contains(PlayerTeamId));
				TeamMemberCounts[PlayerTeamId] += 1;
			}
		}
	}

	// return INDEX_NONE;

	// now sort by lowest team population, then by team ID
	int8 BestTeamId = INDEX_NONE;
	uint32 BestPlayerCount = TNumericLimits<uint32>::Max();
	for (const auto& TeamMemberCount : TeamMemberCounts)
	{
		const int8 TestTeamId = TeamMemberCount.Key;
		const uint32 TestTeamPlayerCount = TeamMemberCount.Value;

		if (TestTeamPlayerCount < BestPlayerCount)
		{
			BestTeamId = TestTeamId;
			BestPlayerCount = TestTeamPlayerCount;
		}
		else if (TestTeamPlayerCount == BestPlayerCount)
		{
			if ((TestTeamId < BestTeamId) || (BestTeamId == INDEX_NONE))
			{
				BestTeamId = TestTeamId;
				BestPlayerCount = TestTeamPlayerCount;
			}
		}
	}
	
	return BestTeamId;
}
