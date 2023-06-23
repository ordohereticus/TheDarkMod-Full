/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2652 $
 * $Date: 2008-07-13 10:33:31 -0400 (Sun, 13 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: Conversation.cpp 2652 2008-07-13 14:33:31Z greebo $", init_version);

#include "Conversation.h"

namespace ai {

Conversation::Conversation() :
	_isValid(false),
	_talkDistance(0.0f)
{}

Conversation::Conversation(const idDict& spawnArgs, int index) :
	_isValid(false),
	_talkDistance(0.0f)
{
	// Pass the call to the parser
	InitFromSpawnArgs(spawnArgs, index);
}

bool Conversation::IsValid()
{
	return _isValid;
}

void Conversation::Save(idSaveGame* savefile) const
{
	savefile->WriteBool(_isValid);
	savefile->WriteFloat(_talkDistance);
}

void Conversation::Restore(idRestoreGame* savefile)
{
	savefile->ReadBool(_isValid);
	savefile->ReadFloat(_talkDistance);
}

void Conversation::InitFromSpawnArgs(const idDict& dict, int index)
{
	idStr prefix = va("conv_%d", index);

	_talkDistance = dict.GetFloat(prefix + "talk_distance");

	// TODO: Add validity check here
	_isValid = false;
}

} // namespace ai
