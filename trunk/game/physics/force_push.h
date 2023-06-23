/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2764 $
 * $Date: 2008-08-30 04:56:32 -0400 (Sat, 30 Aug 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/
#ifndef __FORCE_PUSH_H__
#define __FORCE_PUSH_H__

#include "Force.h"

/**
 * greebo: This class should represent the push force as applied by the player
 * to large game world objects.
 */
class CForcePush : 
	public idForce
{
public:
	CLASS_PROTOTYPE( CForcePush );

						CForcePush();

						// initialize the drag force
	void				Init( float damping );

	void				SetOwner(idEntity* ownerEnt);

	// Set physics object which is about to be pushed
	void				SetPushEntity(idEntity* pushEnt, int id);

	// Set the push parameters for the next evaluation frame
	void				SetContactInfo(const trace_t& contactInfo, const idVec3& impactVelocity);

						// set position to drag towards
	/*void				SetDragPosition( const idVec3 &pos );
						// get the position dragged towards
	const idVec3 &		GetDragPosition( void ) const;
						// get the position on the dragged physics object
	const idVec3		GetDraggedPosition( void ) const;*/

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

public: // common force interface
	virtual void		Evaluate( int time );

private:

	// properties
	//float				damping;

	idEntity*			pushEnt;		// entity being pushed
	int					id;				// clip model id of physics object
	trace_t				contactInfo;	// the contact info of the object we're pushing
	idVec3				impactVelocity;	// the velocity the owner had at impact time
	int					startPushTime;	// the time we started to push the physics object

	idEntity*			owner;			// the owning entity
};

#endif /* __FORCE_PUSH_H__ */
