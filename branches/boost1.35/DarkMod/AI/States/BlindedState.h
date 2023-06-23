/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_BLINDED_STATE_H__
#define __AI_BLINDED_STATE_H__

#include "../../../game/ai/ai.h"
#include "State.h"

namespace ai
{

#define STATE_BLINDED "Blinded"

class BlindedState :
	public State
{
private:
	int _endTime;
	float _oldAcuity;

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
};

} // namespace ai

#endif /* __AI_BLINDED_STATE_H__ */
