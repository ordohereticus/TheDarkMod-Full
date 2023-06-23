/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2664 $
 * $Date: 2008-07-14 15:46:07 -0400 (Mon, 14 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_CONVERSATION_STATE_H__
#define __AI_CONVERSATION_STATE_H__

#include "State.h"

namespace ai
{

#define STATE_CONVERSATION "Conversation"

class ConversationState :
	public State
{
public:
	// Get the name of this state
	virtual const idStr& GetName() const;

	// This is called when the state is first attached to the AI's Mind.
	virtual void Init(idAI* owner);

	// Gets called each time the mind is thinking
	virtual void Think(idAI* owner);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Override base class method
	virtual bool CheckAlertLevel(idAI* owner);

	static StatePtr CreateInstance();
};

} // namespace ai

#endif /* __AI_CONVERSATION_STATE_H__ */
