/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-16 12:53:28 -0400 (Tue, 16 Oct 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __FORCE_GRAB_H__
#define __FORCE_GRAB_H__

#include "../game/physics/force.h"

/*
===============================================================================

	Grab force

===============================================================================
*/

class CForce_Grab : public idForce 
{
	public:
		CLASS_PROTOTYPE( CForce_Grab );

							CForce_Grab( void );
		virtual				~CForce_Grab( void );
							// initialize the drag force
		void				Init( float damping );
							// set physics object being dragged
		void				SetPhysics( idPhysics *physics, int id, const idVec3 &p );
							// set position to drag towards
		void				SetDragPosition( const idVec3 &pos );
							// get the position dragged towards
		const idVec3 &		GetDragPosition( void ) const;
							// get the position on the dragged physics object
		const idVec3		GetDraggedPosition( void ) const;

							// Gets the center of mass of the grabbed object
		idVec3				GetCenterOfMass( void ) const;
							// rotates p about the center of mass of the grabbed object
		void				Rotate( const idVec3 &vec, float angle );

		void				Save( idSaveGame *savefile ) const;
		void				Restore( idRestoreGame *savefile );

		/**
		* Toggle whether the force_grab should use a max force limit
		**/
		void				LimitForce( bool bVal );

		/**
		* Toggle whether the force_grab should apply damping or not
		**/
		void				ApplyDamping( bool bVal );

		/**
		* Set the reference entity
		**/
		void				SetRefEnt( idEntity *ent );

	public: // common force interface
		virtual void		Evaluate( int time );
		virtual void		RemovePhysics( const idPhysics *phys );

	private:

		/**
		* Entity to which this drag force is referenced, if any
		**/
		idEntityPtr<idEntity>	m_RefEnt;

		/**
		* If true, limit force or apply damping
		**/
		bool				m_bLimitForce;
		bool				m_bApplyDamping;

		// properties
		float				m_damping;

		// physics object properties
		idVec3				m_centerOfMass;

		// positioning
		idPhysics *			m_physics;		// physics object
		idVec3				m_p;				// position on clip model
		int					m_id;				// clip model id of physics object
		idVec3				m_dragPosition;	// drag towards this position
		/**
		* Origin of the dragged entity in the previous frame
		**/
		idVec3				m_prevOrigin;
};

#endif /* !__FORCE_GRAB_H__ */
