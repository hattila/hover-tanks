// Fill out your copyright notice in the Description page of Project Settings.


#include "HTGSTeamDeathMatch.h"

#include "HoverTanks/Game/InTeamPlayerState.h"
#include "HoverTanks/Game/Teams/HTTeam.h"
#include "..\Teams\HTTeamDataAsset.h"

AHTGSTeamDeathMatch::AHTGSTeamDeathMatch()
{
	// find and set the team data asset
	static ConstructorHelpers::FObjectFinder<UHTTeamDataAsset> Team1DataAssetFinder(TEXT("/Game/HoverTanks/Game/Teams/DA_RedTeam"));
	if (Team1DataAssetFinder.Succeeded())
	{
		TeamsToCreate.Add(1, Team1DataAssetFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UHTTeamDataAsset> Team2DataAssetFinder(TEXT("/Game/HoverTanks/Game/Teams/DA_BlueTeam"));
	if (Team2DataAssetFinder.Succeeded())
	{
		TeamsToCreate.Add(2, Team2DataAssetFinder.Object);
	}
	
}

void AHTGSTeamDeathMatch::InitializeNewPlayerScore(const APlayerController* NewPlayer)
{
	AInTeamPlayerState* InTeamPlayerState = Cast<AInTeamPlayerState>(NewPlayer->PlayerState);
	if (!InTeamPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddScoreToPlayer : PlayerState is not InTeamPlayerState"));
		Super::InitializeNewPlayerScore(NewPlayer);
		return;
	}

	for (int32 i = 0; i < PlayerScores.Num(); i++)
	{
		if (PlayerScores[i].PlayerName == InTeamPlayerState->GetPlayerName())
		{
			// UE_LOG(LogTemp, Warning, TEXT("InitializeNewPlayerScore : Player already in array. Name: %s, team: %d"), *InTeamPlayerState->GetPlayerName(), InTeamPlayerState->GetTeamId());
			
			PlayerScores[i].TeamId = InTeamPlayerState->GetTeamId();
			OnRep_PlayerScores();
			return;
		}
	}
	
	FPlayerScore PlayerScore;
	PlayerScore.PlayerName = InTeamPlayerState->GetPlayerName();
	PlayerScore.Score = 0;
	PlayerScore.TeamId = InTeamPlayerState->GetTeamId();

	PlayerScores.Add(PlayerScore);

	OnRep_PlayerScores();
}

void AHTGSTeamDeathMatch::AddScoreToPlayer(const APlayerController* PlayerController, const int32 ScoreToAdd)
{
	AInTeamPlayerState* InTeamPlayerState = Cast<AInTeamPlayerState>(PlayerController->PlayerState);
	if (!InTeamPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddScoreToPlayer : PlayerState is not InTeamPlayerState"));
		Super::AddScoreToPlayer(PlayerController, ScoreToAdd);
		return;
	}

	// UE_LOG(LogTemp, Warning, TEXT("AddScoreToPlayer : PlayerState is InTeamPlayerState, finding player and adding teamID"));
	
	for (int32 i = 0; i < PlayerScores.Num(); i++)
	{
		if (PlayerScores[i].PlayerName == InTeamPlayerState->GetPlayerName())
		{
			PlayerScores[i].TeamId = InTeamPlayerState->GetTeamId();
			break;
		}
	}
	
	Super::AddScoreToPlayer(PlayerController, ScoreToAdd);
}

void AHTGSTeamDeathMatch::CreateTeams()
{
	if (TeamMap.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Teams already created"));
		return;
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("Creating Teams"));

	for (const auto& TeamToCreate : TeamsToCreate)
	{
		AHTTeam* NewTeam = GetWorld()->SpawnActor<AHTTeam>(AHTTeam::StaticClass());
		NewTeam->SetTeamId(TeamToCreate.Key);
		NewTeam->SetTeamDataAsset(TeamToCreate.Value);

		TeamMap.Add(TeamToCreate.Key, NewTeam);
	}
}

void AHTGSTeamDeathMatch::AssignPlayersToTeams()
{
	// UE_LOG(LogTemp, Warning, TEXT("AssignPlayersToTeams"));
	
	// for each connected player, cast their player state to InTeamPlayerState, and assign them to a team
	for (APlayerState* PS : PlayerArray)
	{
		if (AInTeamPlayerState* InTeamPS = Cast<AInTeamPlayerState>(PS))
		{
			AssignPlayerToLeastPopulatedTeam(InTeamPS);
		}
	}
}

bool AHTGSTeamDeathMatch::AssignPlayerToLeastPopulatedTeam(AInTeamPlayerState* PlayerState)
{
	int8 LeastPopulatedTeamId = GetLeastPopulatedTeamId();
	if (LeastPopulatedTeamId == INDEX_NONE)
	{
		return false;
	}

	return AssignPlayerToTeam(PlayerState, LeastPopulatedTeamId);
}

bool AHTGSTeamDeathMatch::AssignPlayerToTeam(AInTeamPlayerState* TeamPlayerState, int8 TeamId)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AssignPlayerToTeam : Only callable on the server"));
		return false;
	}

	if (!TeamMap.Contains(TeamId))
	{
		UE_LOG(LogTemp, Warning, TEXT("AssignPlayerToTeam : Invalid team id %d"), TeamId);
		return false;
	}

	TeamPlayerState->SetTeamId(TeamId);
	return true;
}

bool AHTGSTeamDeathMatch::AreSameTeam(APlayerController* PlayerController1, APlayerController* PlayerController2)
{
	const AInTeamPlayerState* PlayerState1 = PlayerController1->GetPlayerState<AInTeamPlayerState>();
	const AInTeamPlayerState* PlayerState2 = PlayerController2->GetPlayerState<AInTeamPlayerState>();

	if (PlayerState1 && PlayerState2)
	{
		return PlayerState1->GetTeamId() == PlayerState2->GetTeamId();
	}
	
	return false;
}

void AHTGSTeamDeathMatch::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	// log
	// UE_LOG(LogTemp, Warning, TEXT("Team Death Match Begin Play, subscribing to OnTeamIdChanged"));
	//
	// // foreach player state, cast to InTeamPlayerState and subscribe to OnTeamIdChanged
	// for (APlayerState* PS : PlayerArray)
	// {
	// 	if (AInTeamPlayerState* InTeamPS = Cast<AInTeamPlayerState>(PS))
	// 	{
	// 		InTeamPS->OnTeamIdChanged.AddDynamic(this, &ATeamDeathMatchGameState::HandleTeamIdChanged);
	// 	}
	// }
}

// void ATeamDeathMatchGameState::HandleTeamIdChanged(int8 NewTeamId)
// {
// 	UE_LOG(LogTemp, Warning, TEXT("Team Death Match HandleTeamIdChanged, calling on rep player scores"));
// 	
// 	OnRep_PlayerScores();
// }

/**
 * Based on the Lyra example
 */
int8 AHTGSTeamDeathMatch::GetLeastPopulatedTeamId() const
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
