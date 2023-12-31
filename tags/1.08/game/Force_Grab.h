/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5121 $ (Revision of last commit) 
 $Date: 2011-12-11 14:12:26 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#ifndef __FORCE_GRAB_H__
#define __FORCE_GRAB_H__

#include "physics/Force.h"

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
