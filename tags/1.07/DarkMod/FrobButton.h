/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4854 $
 * $Date: 2011-05-19 19:52:52 -0400 (Thu, 19 May 2011) $
 * $Author: grayman $
 *
 ***************************************************************************/

#ifndef FROBBUTTON_H
#define FROBBUTTON_H

/** 
 * angua: This class is designed specifically for buttons.
 * It doesn't do much more than using the BinaryFrobMover functions for now
 * but I guess we will want some additional functionality in the future.
 */
class CFrobButton : 
	public CBinaryFrobMover 
{
public:

	CLASS_PROTOTYPE( CFrobButton );

	void					Spawn();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	// this does only call open for now, might be that we want 
	// a more complex operation algorithm when the button is pressed in the future
	virtual void			Operate();

	// Override the base class's ApplyImpulse method
	virtual void			ApplyImpulse(idEntity *ent, int id, const idVec3 &point, const idVec3 &impulse);
};

#endif /* FROBBUTTON_H */
