/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3546 $
 * $Date: 2009-07-18 13:27:48 -0400 (Sat, 18 Jul 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_FLEE_H__
#define __AI_FLEE_H__

#include "State.h"

namespace ai
{

#define STATE_FLEE "Flee"

class FleeState :
	public State
{
public:
	// Get the name of this state
	virtual const idStr& GetName() const;

	// This is called when the state is first attached to the AI's Mind.
	virtual void Init(idAI* owner);

	// Gets called each time the mind is thinking
	virtual void Think(idAI* owner);

	virtual void OnFailedKnockoutBlow(idEntity* attacker, const idVec3& direction, bool hitHead);

	static StatePtr CreateInstance();
};

} // namespace ai

#endif /* __AI_FLEE_H__ */
