/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2442 $
 * $Date: 2008-06-06 12:22:13 -0400 (Fri, 06 Jun 2008) $
 * $Author: angua $
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

	virtual void			Open(bool Master);
	virtual void			Close(bool Master);

	// Override the base class's ApplyImpulse method
	virtual void			ApplyImpulse( idEntity *ent, int id, const idVec3 &point, const idVec3 &impulse );

	virtual void			ClosePortal();
	virtual void			OpenPortal();

};

#endif /* FROBBUTTON_H */
