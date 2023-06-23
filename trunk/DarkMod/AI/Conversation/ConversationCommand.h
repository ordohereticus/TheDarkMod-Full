/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2658 $
 * $Date: 2008-07-13 14:16:35 -0400 (Sun, 13 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_CONVERSATION_COMMAND_H__
#define __AI_CONVERSATION_COMMAND_H__

#include "../../../idlib/precompiled.h"

#include <boost/shared_ptr.hpp>

namespace ai {

class ConversationCommand
{
public:
	/**
	 * greebo: Parses the command parameters from the given idDict.
	 * The prefix is something along the lines "conv_2_cmd_3_" and is
	 * used to find all spawnargs relevant for this command.
	 *
	 * Returns TRUE if the parse process succeeded, FALSE otherwise.
	 */
	bool Parse(const idDict& dict, const idStr& prefix);

	// Save/Restore routines
	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);
};
typedef boost::shared_ptr<ConversationCommand> ConversationCommandPtr;

} // namespace ai

#endif /* __AI_CONVERSATION_COMMAND_H__ */
