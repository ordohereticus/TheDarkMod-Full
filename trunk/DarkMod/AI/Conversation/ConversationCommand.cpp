/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2658 $
 * $Date: 2008-07-13 14:16:35 -0400 (Sun, 13 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ConversationCommand.cpp 2658 2008-07-13 18:16:35Z greebo $", init_version);

#include "ConversationCommand.h"

namespace ai {

bool ConversationCommand::Parse(const idDict& dict, const idStr& prefix)
{
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

} // namespace ai
