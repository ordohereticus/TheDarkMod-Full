/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 2 $
 * $Date: 2004-10-30 11:52:07 -0400 (Sat, 30 Oct 2004) $
 * $Author: sparhawk $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:33  sparhawk
 * Initial revision
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __FORCE_CONSTANT_H__
#define __FORCE_CONSTANT_H__

/*
===============================================================================

	Constant force

===============================================================================
*/

class idForce_Constant : public idForce {

public:
	CLASS_PROTOTYPE( idForce_Constant );

						idForce_Constant( void );
	virtual				~idForce_Constant( void );


	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

						// constant force
	void				SetForce( const idVec3 &force );
						// set force position
	void				SetPosition( idPhysics *physics, int id, const idVec3 &point );

	void				SetPhysics( idPhysics *physics );

public: // common force interface
	virtual void		Evaluate( int time );
	virtual void		RemovePhysics( const idPhysics *phys );

private:
	// force properties
	idVec3				force;
	idPhysics *			physics;
	int					id;
	idVec3				point;
};

#endif /* !__FORCE_CONSTANT_H__ */
