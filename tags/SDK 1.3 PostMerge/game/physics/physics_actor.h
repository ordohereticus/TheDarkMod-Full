/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 866 $
 * $Date: 2007-03-23 17:25:02 -0400 (Fri, 23 Mar 2007) $
 * $Author: sparhawk $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __PHYSICS_ACTOR_H__
#define __PHYSICS_ACTOR_H__

/*
===================================================================================

	Actor physics base class

	An actor typically uses one collision model which is aligned with the gravity
	direction. The collision model is usually a simple box with the origin at the
	bottom center.

===================================================================================
*/

#ifdef MOD_WATERPHYSICS
typedef enum {					// MOD_WATERPHYSICS
	WATERLEVEL_NONE,			// MOD_WATERPHYSICS
	WATERLEVEL_FEET,			// MOD_WATERPHYSICS
	WATERLEVEL_WAIST,			// MOD_WATERPHYSICS
	WATERLEVEL_HEAD				// MOD_WATERPHYSICS
} waterLevel_t;					// MOD_WATERPHYSICS
#endif // MOD_WATERPHYSICS

class idPhysics_Actor : public idPhysics_Base {

public:
	CLASS_PROTOTYPE( idPhysics_Actor );

							idPhysics_Actor( void );
							~idPhysics_Actor( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

							// get delta yaw of master
	float					GetMasterDeltaYaw( void ) const;
							// returns the ground entity
	idEntity *				GetGroundEntity( void ) const;
							// align the clip model with the gravity direction
	void					SetClipModelAxis( void );

#ifdef MOD_WATERPHYSICS
	virtual waterLevel_t	GetWaterLevel( void ) const; 	// MOD_WATERPHYSICS
	virtual int				GetWaterType( void ) const; 	// MOD_WATERPHYSICS
#endif

public:	// common physics interface
	void					SetClipModel( idClipModel *model, float density, int id = 0, bool freeOld = true );
	idClipModel *			GetClipModel( int id = 0 ) const;
	int						GetNumClipModels( void ) const;

	void					SetMass( float mass, int id = -1 );
	float					GetMass( int id = -1 ) const;

	void					SetContents( int contents, int id = -1 );
	int						GetContents( int id = -1 ) const;

	const idBounds &		GetBounds( int id = -1 ) const;
	const idBounds &		GetAbsBounds( int id = -1 ) const;

	bool					IsPushable( void ) const;

	const idVec3 &			GetOrigin( int id = 0 ) const;
	const idMat3 &			GetAxis( int id = 0 ) const;

	void					SetGravity( const idVec3 &newGravity );
	const idMat3 &			GetGravityAxis( void ) const;

	void					ClipTranslation( trace_t &results, const idVec3 &translation, const idClipModel *model ) const;
	void					ClipRotation( trace_t &results, const idRotation &rotation, const idClipModel *model ) const;
	int						ClipContents( const idClipModel *model ) const;

	void					DisableClip( void );
	void					EnableClip( void );

	void					UnlinkClip( void );
	void					LinkClip( void );

	bool					EvaluateContacts( void );

protected:
#ifdef MOD_WATERPHYSICS
	virtual void		SetWaterLevel( void );		// MOD_WATERPHYSICS
	waterLevel_t		waterLevel;					// MOD_WATERPHYSICS
	int					waterType;					// MOD_WATERPHYSICS
#endif

	idClipModel *			clipModel;			// clip model used for collision detection
	idMat3					clipModelAxis;		// axis of clip model aligned with gravity direction

	// derived properties
	float					mass;
	float					invMass;

	// master
	idEntity *				masterEntity;
	float					masterYaw;
	float					masterDeltaYaw;

	// results of last evaluate
	idEntityPtr<idEntity>	groundEntityPtr;
};

#endif /* !__PHYSICS_ACTOR_H__ */
