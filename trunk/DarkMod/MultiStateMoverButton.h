/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2340 $
 * $Date: 2008-05-15 13:28:07 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __MULTISTATEMOVER_BUTTON_H_
#define __MULTISTATEMOVER_BUTTON_H_

#include "FrobButton.h"

enum EMMButtonType
{
	BUTTON_TYPE_RIDE = 0,
	BUTTON_TYPE_FETCH,
	NUM_BUTTON_TYPES,
};

/** 
 * greebo: A MultiStateMoverButton is a bit more intelligent
 * than an ordinary FrobButton as it is "communicating" with
 * the targetted elevator a bit at spawn time.
 */
class CMultiStateMoverButton : 
	public CFrobButton 
{
public:

	CLASS_PROTOTYPE( CMultiStateMoverButton );

	void					Spawn();

private:
	void					Event_PostSpawn();
};

#endif /* __MULTISTATEMOVER_BUTTON_H_ */
