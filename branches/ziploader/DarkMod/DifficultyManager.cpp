/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2959 $
 * $Date: 2008-10-20 11:46:29 -0400 (Mon, 20 Oct 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "DifficultyManager.h"

namespace difficulty {

// Constructor
DifficultyManager::DifficultyManager() :
	_difficulty(0)
{}

void DifficultyManager::Clear()
{
	for (int i = 0; i < DIFFICULTY_COUNT; i++)
	{
		_globalSettings[i].Clear();
		_difficultyNames[i] = "";
	}
}

void DifficultyManager::Init(idMapFile* mapFile)
{
	DM_LOG(LC_DIFFICULTY, LT_INFO)LOGSTRING("Searching for difficulty setting on worldspawn.\r");

	if (mapFile->GetNumEntities() <= 0) {
		return; // no entities!
	}

	// Fetch the worldspawn
	idMapEntity* mapEnt = mapFile->GetEntity(0);
	idDict spawnArgs = mapEnt->epairs;

	int mapDifficulty;
	if (spawnArgs.GetInt("difficulty", "0", mapDifficulty))
	{
		// We have a difficulty spawnarg set on the map's worldspawn, take it as override value
		DM_LOG(LC_DIFFICULTY, LT_DEBUG)LOGSTRING("Found overriding difficulty setting on worldspawn entity: %d.\r", mapDifficulty);
		_difficulty = mapDifficulty;
	}

	// Check for the CVAR, which might override any setting
	if (cv_tdm_difficulty.GetInteger() >= 0)
	{
		_difficulty = cv_tdm_difficulty.GetInteger();
		DM_LOG(LC_DIFFICULTY, LT_DEBUG)LOGSTRING("Found overriding CVAR 'tdm_difficulty': %d.\r", _difficulty);
	}

	// Load the default difficulty settings from the entityDefs
	LoadDefaultDifficultySettings();

	LoadMapDifficultySettings(mapFile);
}

void DifficultyManager::SetDifficultyLevel(int difficulty)
{
	_difficulty = difficulty;
}

int DifficultyManager::GetDifficultyLevel() const
{
	return _difficulty;
}

const idStr& DifficultyManager::GetDifficultyName(int level)
{
	assert(level >= 0 && level < DIFFICULTY_COUNT);
	return _difficultyNames[level];
}

void DifficultyManager::Save(idSaveGame* savefile)
{
	savefile->WriteInt(_difficulty);
	for (int i = 0; i < DIFFICULTY_COUNT; i++)
	{
		_globalSettings[i].Save(savefile);
	}
}

void DifficultyManager::Restore(idRestoreGame* savefile)
{
	savefile->ReadInt(_difficulty);
	for (int i = 0; i < DIFFICULTY_COUNT; i++)
	{
		_globalSettings[i].Restore(savefile);
	}
}

void DifficultyManager::ApplyDifficultySettings(idDict& target)
{
	DM_LOG(LC_DIFFICULTY, LT_INFO)LOGSTRING("Applying difficulty settings to entity: %s.\r", target.GetString("name"));

	// greebo: Preliminary case: just apply the global settings
	_globalSettings[_difficulty].ApplySettings(target);
}

bool DifficultyManager::InhibitEntitySpawn(const idDict& target) {
	bool isAllowed(true);

	// Construct the key ("diff_0_spawn")
	idStr key = va("diff_%d_nospawn", _difficulty);

	// The entity is allowed to spawn by default, must be set to 1 by the mapper
	isAllowed = !target.GetBool(key, "0");

	DM_LOG(LC_DIFFICULTY, LT_INFO)LOGSTRING("Entity %s is allowed to spawn: %s.\r", target.GetString("name"), isAllowed ? "YES" : "NO");

	// Return false if the entity is allowed to spawn
	return !isAllowed;
}

void DifficultyManager::LoadDefaultDifficultySettings()
{
	DM_LOG(LC_DIFFICULTY, LT_INFO)LOGSTRING("Trying to load default difficulty settings from entityDefs.\r");

	// Construct the entityDef name (e.g. atdm:difficulty_settings_default_0)
	idStr defName(DEFAULT_DIFFICULTY_ENTITYDEF);

	const idDict* difficultyDict = gameLocal.FindEntityDefDict(defName);

	if (difficultyDict != NULL)
	{
		DM_LOG(LC_DIFFICULTY, LT_DEBUG)LOGSTRING("Found difficulty settings: %s.\r", defName.c_str());

		// greebo: Try to lookup the entityDef for each difficulty level and load the settings
		for (int i = 0; i < DIFFICULTY_COUNT; i++)
		{
			// Let the setting structure know which level it is referring to
			_globalSettings[i].SetLevel(i);
			// And load the settings
			_globalSettings[i].LoadFromEntityDef(*difficultyDict);
		}
	}
	else
	{
		for (int i = 0; i < DIFFICULTY_COUNT; i++)
		{
			_globalSettings[i].Clear();
		}
		gameLocal.Warning("DifficultyManager: Could not find default difficulty entityDef!");
	}
}

void DifficultyManager::LoadMapDifficultySettings(idMapFile* mapFile)
{
	DM_LOG(LC_DIFFICULTY, LT_INFO)LOGSTRING("Trying to load map-specific difficulty settings.\r");

	if (mapFile == NULL) return;

	for (int i = 0; i < mapFile->GetNumEntities(); i++)
	{
		idMapEntity* ent = mapFile->GetEntity(i);

		if (idStr::Icmp(ent->epairs.GetString("classname"), DIFFICULTY_ENTITYDEF) == 0)
		{
			LoadMapDifficultySettings(ent);
		}
	}

	// greebo: Find out the names of the difficulty settings
	idMapEntity* worldSpawn = mapFile->GetEntity(0);
	idDict mapDict = worldSpawn->epairs;

	// Determine the difficulty level string. The defaults are the "difficultyMenu" entityDef.
	// Maps can override these values by use of the difficulty#Name value on the spawnargs of 
	// the worldspawn.
	const idDecl* diffDecl = declManager->FindType(DECL_ENTITYDEF, "difficultyMenu", false);
	const idDeclEntityDef* diffDef = static_cast<const idDeclEntityDef*>(diffDecl);
	for (int diffLevel = 0; diffLevel < DIFFICULTY_COUNT; diffLevel++)
	{
		_difficultyNames[diffLevel] = mapDict.GetString(
			va("difficulty%dName",diffLevel),
			diffDef->dict.GetString(va("diff%ddefault",diffLevel), "")
		);
	}
}

void DifficultyManager::LoadMapDifficultySettings(idMapEntity* ent)
{
	if (ent == NULL) return;

	// greebo: Let each global settings structure investigate the settings 
	// on this entity.
	for (int i = 0; i < DIFFICULTY_COUNT; i++)
	{
		_globalSettings[i].LoadFromMapEntity(ent);
	}
}

} // namespace difficulty
