/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2661 $
 * $Date: 2008-07-14 00:46:02 -0400 (Mon, 14 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ConversationCommand.cpp 2661 2008-07-14 04:46:02Z greebo $", init_version);

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
	_type = GetType(dict.GetString(prefix + "type", idStr(ENumCommands)));

	if (_type == ENumCommands)
	{
		DM_LOG(LC_CONVERSATION, LT_DEBUG)LOGSTRING("Could not find type for command prefix %s.\r", prefix.c_str());
		return false; // invalid command type
	}

	DM_LOG(LC_CONVERSATION, LT_DEBUG)LOGSTRING("Found command type %s for prefix %s.\r", TypeNames[_type], prefix.c_str());

	// Parse the arguments
	_arguments.Clear();

	idStr argPrefix = prefix + "arg_";
	for (const idKeyValue* kv = dict.MatchPrefix(argPrefix); kv != NULL; kv = dict.MatchPrefix(argPrefix, kv))
	{
		// Add each actor name to the list
		DM_LOG(LC_CONVERSATION, LT_DEBUG)LOGSTRING("Adding argument %s to conversation command %s.\r", kv->GetValue().c_str(), TypeNames[_type]);
		_arguments.Append(kv->GetValue());
	}

	DM_LOG(LC_CONVERSATION, LT_DEBUG)LOGSTRING("Command type %s has %d arguments.\r", TypeNames[_type], _arguments.Num());

	return true;
}

void ConversationCommand::Save(idSaveGame* savefile) const
{
	savefile->WriteInt(static_cast<int>(_type));
}

void ConversationCommand::Restore(idRestoreGame* savefile)
{
	int typeInt;
	savefile->ReadInt(typeInt);
	assert(typeInt >= 0 && typeInt <= ENumCommands); // sanity check
	_type = static_cast<Type>(typeInt);
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
