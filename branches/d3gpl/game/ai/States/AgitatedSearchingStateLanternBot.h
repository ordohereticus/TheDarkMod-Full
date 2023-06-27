/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5088 $
 * $Date: 2011-12-05 02:07:27 -0500 (Mon, 05 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _AGITATED_SEARCHING_STATE_LANTERN_BOT_H_
#define _AGITATED_SEARCHING_STATE_LANTERN_BOT_H_

#include "../AI.h"
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
