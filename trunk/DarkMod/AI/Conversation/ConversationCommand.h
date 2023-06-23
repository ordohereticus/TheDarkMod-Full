/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2657 $
 * $Date: 2008-07-13 13:35:13 -0400 (Sun, 13 Jul 2008) $
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
	// Save/Restore routines
	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);
};
typedef boost::shared_ptr<ConversationCommand> ConversationCommandPtr;

} // namespace ai

#endif /* __AI_CONVERSATION_COMMAND_H__ */
