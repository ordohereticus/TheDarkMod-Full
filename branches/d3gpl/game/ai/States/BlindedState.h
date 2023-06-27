/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5088 $
 * $Date: 2011-12-05 02:07:27 -0500 (Mon, 05 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_BLINDED_STATE_H__
#define __AI_BLINDED_STATE_H__

#include "../AI.h"
#include "State.h"

namespace ai
{

#define STATE_BLINDED "Blinded"

class BlindedState :
	public State
{
private:
	int _endTime;
	float _oldAcuity;	// to restore visual acuity

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
