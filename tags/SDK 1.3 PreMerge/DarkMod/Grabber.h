/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 866 $
 * $Date: 2007-03-23 17:25:02 -0400 (Fri, 23 Mar 2007) $
 * $Author: sparhawk $
 *
 ***************************************************************************/

#ifndef __GRABBER_H__
#define __GRABBER_H__

#include "../Game/Entity.h"
#include "Force_Grab.h"

class idPlayer;

extern const idEventDef EV_Grabber_CheckClipList;

class CGrabbedEnt 
{
	public: 
		idEntity	*m_ent;
		int			m_clipMask;
		int			m_contents;

		bool operator==( const CGrabbedEnt &right ) const 
		{
			if( right.m_ent == this->m_ent )
				return true;
			return false;
		}
};

class CGrabber : public idEntity {
public:
		CLASS_PROTOTYPE( CGrabber );


								CGrabber( void );
								~CGrabber( void );

		void					Clear( void );
		void					Update( idPlayer *player, bool hold = false );

		void					Spawn( void );

		idEntity *				GetSelected( void ) const { return m_dragEnt.GetEntity(); }

		bool					IsInClipList( idEntity *ent ) const;
		bool					HasClippedEntity( void ) const;

		/**
		* Clamp the current velocity to max velocity
		**/
		void					ClampVelocity( float maxLin, float maxAng, int idVal = 0 );

		/**
		* Increment the distance at which the held item is held.
		* Decrements the distance if the argument is false
		**/
		void					IncrementDistance( bool bIncrease );

		/**
		* Attempts to teleport an entity to the minimum hold distance and start holding it
		* Returns false and does nothing if there was not space to teleport in the entity
		* Body ID may be optionally set for AF or multi-clipmodel entities
		*
		* NOTE: Rotation of the entity to the desired orientation should be done externally
		* before calling this.
		**/
		bool					PutInHands( idEntity *ent, idPlayer *player, int bodyID = 0 );

public:
		/**
		* Set to true if the grabbed entity is colliding this frame
		**/
		bool					m_bIsColliding;

		/**
		* Set to true if the grabbed entity collided in the previous frame
		**/
		bool					m_bPrevFrameCollided;

protected:

		/**
		* Start grabbing an item.  Called internally and by the inventory
		* 
		* If newEnt argument is NULL, it tries to grab the entity the player is frobbing
		* Otherwise it places newEnt in the hands.
		*
		* Also calls StopDrag to drop the current item before grabbing the new one, 
		* but we may need to put a time delay between so that we don't have 
		* Pauli Exclusion issues.
		**/
		void					StartDrag( idPlayer *player, idEntity *newEnt = NULL, int bodyID = 0 );

		/**
		* Performs object rotation
		* Also turns the item to face the player when they yaw their view.
		**/
		void					ManipulateObject( idPlayer *player );
		
		void					AddToClipList( idEntity *ent );
		void					RemoveFromClipList( int index );

		void					Event_CheckClipList( void );

		/**
		* Throws the current item.
		* Argument is the amount of time the throw button has been held,
		* used to determine strength of the throw
		**/
		void					Throw( int HeldTime );

		/**
		* Stop dragging and drop the current item if there is one
		**/
		void					StopDrag( void );
		
		/**
		*  returns true if the mouse is inside the dead zone
		**/
		bool					DeadMouse( void );

protected:

		idEntityPtr<idEntity>	m_dragEnt;			// entity being dragged
		jointHandle_t			m_joint;				// joint being dragged
		int						m_id;					// id of body being dragged
		idVec3					m_localEntityPoint;	// dragged point in entity space

		idPlayer				*m_player;
		CForce_Grab				m_drag;

		idRotation				m_rotation;
		int						m_rotationAxis;		// 0 = none, 1 = x, 2 = y, 3 = z
		idVec2					m_mousePosition;		// mouse position when user pressed BUTTON_ZOOM

		idList<CGrabbedEnt>		m_clipList;

		/**
		* Set to true if the attack button has been pressed (used by throwing)
		**/
		bool					m_bAttackPressed;
		/**
		* Timestamp of when attack button was last pressed (used by throwing)
		**/
		int						m_ThrowTimer;

		/**
		* Time stamp in milliseconds for body drag vertical velocity clamp timer
		* (Actual time is read from cvar)
		* Velocity ramps up to normal the longer the body maintains ground contact
		* Resets when they lose ground contact.
		**/
		int						m_DragUpTimer;

		/**
		* Used to limit lifting dragged bodies off ground
		**/
		float					m_AFBodyLastZ;

		/**
		* Set to true when the body with ground checking is considered off the ground
		**/
		bool					m_bAFOffGround;

		/**
		* Int storing the distance increments for held item distance.
		* When this is equal to m_MaxDistCount, it is held at the maximum
		* distance (the frob distance).
		**/
		int						m_DistanceCount;

		/**
		* Maximum distance increments at which the item can be held.
		* Corresponds to 1.0 * the max distance.
		**/
		int						m_MaxDistCount;

		/**
		* Minimum held distance (can be unique to each entity)
		* NOTE: The maximum held distance is always the frob distance of that ent
		**/
		int						m_MinHeldDist;

		/**
		* When colliding, the held distance gets locked to +/- two increments
		* around this value.  This is to prevent the player from going way
		* past the collision point and having to increment all the way back
		* before they see any response.
		**/
		int						m_LockedHeldDist;
};


#endif /* !__GRABBER_H__ */