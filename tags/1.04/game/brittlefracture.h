/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3327 $
 * $Date: 2009-03-27 23:54:15 -0400 (Fri, 27 Mar 2009) $
 * $Author: ishtvan $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_BRITTLEFRACTURE_H__
#define __GAME_BRITTLEFRACTURE_H__


/*
===============================================================================

B-rep Brittle Fracture - Static entity using the boundary representation
of the render model which can fracture.

===============================================================================
*/

extern const idEventDef EV_UpdateSoundLoss;
extern const idEventDef EV_DampenSound;

typedef struct shard_s {
	idClipModel *				clipModel;
	idFixedWinding				winding;
	idList<idFixedWinding *>	decals;
	idList<bool>				edgeHasNeighbour;
	idList<struct shard_s *>	neighbours;
	idPhysics_RigidBody			physicsObj;
	int							droppedTime;
	bool						atEdge;
	int							islandNum;
} shard_t;


class idBrittleFracture : public idEntity {

public:
	CLASS_PROTOTYPE( idBrittleFracture );

								idBrittleFracture( void );
	virtual						~idBrittleFracture( void );

	void						Save( idSaveGame *savefile ) const;
	void						Restore( idRestoreGame *savefile );

	void						Spawn( void );

	virtual void				Present( void );
	virtual void				Think( void );
	virtual void				ApplyImpulse( idEntity *ent, int id, const idVec3 &point, const idVec3 &impulse );
	virtual void				AddForce( idEntity *ent, int id, const idVec3 &point, const idVec3 &force );
	virtual void				AddDamageEffect( const trace_t &collision, const idVec3 &velocity, const char *damageDefName );
	virtual void				Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );

	void						ProjectDecal( const idVec3 &point, const idVec3 &dir, const int time, const char *damageDefName );
	bool						IsBroken( void ) const;

	enum {
		EVENT_PROJECT_DECAL = idEntity::EVENT_MAXEVENTS,
		EVENT_SHATTER,
		EVENT_MAXEVENTS
	};

	virtual void				ClientPredictionThink( void );
	virtual bool				ClientReceiveEvent( int event, int time, const idBitMsg &msg );

private:
	// setttings
	const idMaterial *			material;
	const idMaterial *			decalMaterial;
	float						decalSize;
	float						maxShardArea;
	float						maxShatterRadius;
	float						minShatterRadius;
	float						linearVelocityScale;
	float						angularVelocityScale;
	float						shardMass;
	float						density;
	float						friction;
	float						bouncyness;
	idStr						fxFracture;

	// state
	idPhysics_StaticMulti		physicsObj;
	idList<shard_t *>			shards;
	idBounds					bounds;
	bool						disableFracture;

	/** TDM: Moss arrow dampens sound of shattering **/
	bool						m_bSoundDamped;

	// for rendering
	mutable int					lastRenderEntityUpdate;
	mutable bool				changed;

	/**
	* Contains the visportal handle that the breakable is touching, if portal is present
	* If no portal is present, is set to 0.
	**/
	qhandle_t					m_AreaPortal;

	bool						UpdateRenderEntity( renderEntity_s *renderEntity, const renderView_t *renderView ) const;
	static bool					ModelCallback( renderEntity_s *renderEntity, const renderView_t *renderView );

	void						AddShard( idClipModel *clipModel, idFixedWinding &w );
	void						RemoveShard( int index );
	void						DropShard( shard_t *shard, const idVec3 &point, const idVec3 &dir, const float impulse, const int time );
	void						Shatter( const idVec3 &point, const idVec3 &impulse, const int time );
	void						DropFloatingIslands( const idVec3 &point, const idVec3 &impulse, const int time );
	void						Break( void );
	void						Fracture_r( idFixedWinding &w );
	void						CreateFractures( const idRenderModel *renderModel );
	void						FindNeighbours( void );

	void						Event_Activate( idEntity *activator );
	void						Event_Touch( idEntity *other, trace_t *trace );
	void						Event_DampenSound( bool bDampen );

	/**
	* Update soundprop to set losses in associated portal, if portal is present
	* Called on spawn and when it breaks
	**/
	void						UpdateSoundLoss( void );
};

#endif /* !__GAME_BRITTLEFRACTURE_H__ */
