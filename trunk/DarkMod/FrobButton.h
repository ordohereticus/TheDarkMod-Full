/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
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
	void					Operate();

	void					Open(bool Master);
	void					Close(bool Master);
};

#endif /* FROBBUTTON_H */
