/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2652 $
 * $Date: 2008-07-13 10:33:31 -0400 (Sun, 13 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_CONVERSATION_SYSTEM_H__
#define __AI_CONVERSATION_SYSTEM_H__

#include "../../../idlib/precompiled.h"

#include "Conversation.h"

namespace ai {

class ConversationSystem
{
	// The indexed list of conversations
	idList<ConversationPtr> _conversations;

public:
	// Clears and removes all allocated data
	void Clear();

	/**
	 * greebo: Initialises this class. This means loading the conversation entities
	 * containing all conversations for this map.
	 */
	void Init(idMapFile* mapFile);

	// Save/Restore routines
	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);

private:
	// Helper to load a conversation from an entity's spawnargs
	void LoadConversationEntity(idMapEntity* entity);
};
typedef boost::shared_ptr<ConversationSystem> ConversationSystemPtr;

} // namespace ai

#endif /* __AI_CONVERSATION_SYSTEM_H__ */
