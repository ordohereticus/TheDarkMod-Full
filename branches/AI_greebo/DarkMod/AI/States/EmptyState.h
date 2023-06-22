/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-16 18:53:28 +0200 (Di, 16 Okt 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_EMPTY_STATE_H__
#define __AI_EMPTY_STATE_H__

#include "State.h"

namespace ai
{

class EmptyState :
	public State
{
public:
	// This is called when the state is first attached to the AI's Mind.
	virtual void Init();
};

} // namespace ai

#endif /* __AI_EMPTY_STATE_H__ */
