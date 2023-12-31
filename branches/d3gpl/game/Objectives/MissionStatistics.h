/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5121 $ (Revision of last commit) 
 $Date: 2011-12-11 14:12:26 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#ifndef MISSIONSTATISTICS_H
#define MISSIONSTATISTICS_H

#include "../../idlib/precompiled.h"

#include "Objective.h" // for objective state enum
#include "../Inventory/LootType.h" // for loot type enum

// Maximum array sizes:
#define MAX_TEAMS 64
#define MAX_TYPES 16
#define MAX_AICOMP 16
#define MAX_ALERTLEVELS 16

struct SStat
{
	int Overall;
	int ByTeam[ MAX_TEAMS ];
	int ByType[ MAX_TYPES ];
	int ByInnocence[2];
	int WhileAirborne;

	SStat() 
	{
		Clear();
	}

	void Clear();
};

/**
* Mission stats: Keep track of everything except for loot groups, which are tracked by the inventory
**/
class MissionStatistics
{
public:
	// AI Stats:
	SStat AIStats[ MAX_AICOMP ];
	
	SStat AIAlerts[ MAX_ALERTLEVELS ];

	int DamageDealt;
	int DamageReceived;
	int HealthReceived;
	int PocketsPicked;

	// Item stats are handled by the inventory, not here, 
	// Might need this for copying over to career stats though
	int FoundLoot[LOOT_COUNT];

	// greebo: This is the available amount of loot in the mission
	int LootInMission[LOOT_COUNT];

	// This gets read out right at "mission complete" time, is 0 before
	unsigned int TotalGamePlayTime;

	// Use an array to store the objective states after mission complete
	// We need the historic state data to handle conditional objectives.
	// This list will be empty throughout the mission, and is filled on mission complete
	idList<EObjCompletionState> ObjectiveStates;

	// grayman #2887 - for tracking how often and for how long the player was seen
	int numberTimesPlayerSeen;
	int totalTimePlayerSeen;

	MissionStatistics() 
	{
		Clear();
	}

	void Clear();

	// Returns the state of the objective specified by the (0-based) index
	// Will return INVALID if the objective index is out of bounds or no data is available
	EObjCompletionState GetObjectiveState(int objNum) const;

	// Store the objective state into the ObjectiveStates array
	void SetObjectiveState(int objNum, EObjCompletionState state);

	// Returns the sum of all found loot types (gold+jewels+goods)
	int GetFoundLootValue() const;

	// Returns the total of all loot types in the mission (gold+jewels+goods)
	int GetTotalLootInMission() const;

	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);
};

#endif /* MISSIONSTATISTICS_H */
