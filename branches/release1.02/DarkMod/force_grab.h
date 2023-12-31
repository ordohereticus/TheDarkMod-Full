/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1450 $
 * $Date: 2007-10-18 10:08:41 -0400 (Thu, 18 Oct 2007) $
 * $Author: ishtvan $
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
		/**
		* Set/get orientation to rotate toward
		**/
		void				SetDragAxis( const idMat3 &Axis );
		idMat3				GetDragAxis( void );

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

	protected:

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
		idMat3				m_inertiaTensor;

		// positioning
		idPhysics *			m_physics;		// physics object
		idVec3				m_p;				// position on clip model
		int					m_id;				// clip model id of physics object
		/**
		*  drag towards this position
		**/
		idVec3				m_dragPosition;
		/**
		* Rotate toward this orientation
		**/
		idMat3				m_dragAxis;
		
		/**
		* Origin of the dragged entity in the previous frame
		**/
		idVec3				m_prevOrigin;
};

#endif /* !__FORCE_GRAB_H__ */
