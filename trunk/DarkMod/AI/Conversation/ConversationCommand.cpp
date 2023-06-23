/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2659 $
 * $Date: 2008-07-13 14:32:50 -0400 (Sun, 13 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ConversationCommand.cpp 2659 2008-07-13 18:32:50Z greebo $", init_version);

#include "ConversationCommand.h"

namespace ai {

// These are the various command type strings, needed for parsing
const char* const ConversationCommand::TypeNames[ConversationCommand::ENumCommands] =
{
	"WaitSeconds",
	"WaitForTrigger",
	"WaitForActor",
	"WalkToPosition",
	"WalkToEntity",
	"StopMove",
	"Talk",
	"PlayAnimOnce",
	"PlayAnimCycle",
	"ActivateTarget",
	"LookAtActor",
	"LookAtPosition",
	"LookAtEntity",
	"TurnToActor",
	"TurnToPosition",
	"TurnToEntity",
	"AttackActor",
	"AttackEntity"
};

bool ConversationCommand::Parse(const idDict& dict, const idStr& prefix)
{
	// Get the type

	return false;
}

void ConversationCommand::Save(idSaveGame* savefile) const
{
	// TODO
}

void ConversationCommand::Restore(idRestoreGame* savefile)
{
	// TODO
}

ConversationCommand::Type ConversationCommand::GetType(const idStr& cmdString)
{
	for (int i = 0; i < ENumCommands; i++)
	{
		if (cmdString == TypeNames[i])
		{
			return static_cast<Type>(i);
		}
	}

	return ENumCommands;
}

} // namespace ai
