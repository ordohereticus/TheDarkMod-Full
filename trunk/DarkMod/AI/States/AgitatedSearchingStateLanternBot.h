/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3865 $
 * $Date: 2010-03-21 07:06:10 -0400 (Sun, 21 Mar 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _AGITATED_SEARCHING_STATE_LANTERN_BOT_H_
#define _AGITATED_SEARCHING_STATE_LANTERN_BOT_H_

#include "../../../game/ai/ai.h"
#include "AgitatedSearchingState.h"

namespace ai
{

#define STATE_AGITATED_SEARCHING_LANTERN_BOT "AgitatedSearchingLanternBot"

class AgitatedSearchingStateLanternBot :
	public AgitatedSearchingState
{
protected:
	idVec3 _curAlertPos;

public:
	// Get the name of this state
	virtual const idStr& GetName() const;

	// This is called when the state is first attached to the AI's Mind.
	virtual void Init(idAI* owner);

	// Gets called each time the mind is thinking
	virtual void Think(idAI* owner);

	static StatePtr CreateInstance();

	// Override base class signal
	virtual void OnMovementBlocked(idAI* owner);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

protected:
	// Override base class method
	virtual bool CheckAlertLevel(idAI* owner);

	void MoveTowardAlertPos(idAI* owner);
};

} // namespace

#endif
