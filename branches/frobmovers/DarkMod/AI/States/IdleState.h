/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2475 $
 * $Date: 2008-06-14 02:37:18 -0400 (Sat, 14 Jun 2008) $
 * $Author: angua $
 *
 ***************************************************************************/

#ifndef __AI_IDLE_STATE_H__
#define __AI_IDLE_STATE_H__

#include "State.h"

namespace ai
{

#define STATE_IDLE "Idle"

class IdleState :
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

	static StatePtr CreateInstance();

protected:

	bool _startSitting;

	// Override base class method
	virtual bool CheckAlertLevel(idAI* owner);

	// Returns the initial idle bark sound, depending on the alert level 
	// and the current state of mind
	virtual idStr GetInitialIdleBark(idAI* owner);

	virtual void InitialiseMovement(idAI* owner);
	virtual void InitialiseCommunication(idAI* owner);
};

} // namespace ai

#endif /* __AI_IDLE_STATE_H__ */
